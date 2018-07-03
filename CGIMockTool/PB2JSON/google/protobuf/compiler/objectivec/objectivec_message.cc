// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.
// http://code.google.com/p/protobuf/
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Author: cyrusn@google.com (Cyrus Najmabadi)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include <algorithm>
#include <google/protobuf/stubs/hash.h>
#include <google/protobuf/compiler/objectivec/objectivec_message.h>
#include <google/protobuf/compiler/objectivec/objectivec_enum.h>
#include <google/protobuf/compiler/objectivec/objectivec_extension.h>
#include <google/protobuf/compiler/objectivec/objectivec_helpers.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.pb.h>

namespace google { namespace protobuf { namespace compiler { namespace objectivec {
    
    using internal::WireFormat;
    using internal::WireFormatLite;
    
    namespace {
        struct FieldOrderingByNumber {
            inline bool operator()(const FieldDescriptor* a,
                                   const FieldDescriptor* b) const {
                return a->number() < b->number();
            }
        };
        
        struct FieldOrderingByType {
            inline bool operator()(const FieldDescriptor* a, const FieldDescriptor* b) const {
                // place collections at the end
                if (a->is_repeated() != b->is_repeated()) {
                    return b->is_repeated();
                }
                
                // we want BOOL fields to be placed first.  That way they will be packed
                // in with the 'BOOL hasFoo' fields.
                if (a->type() == FieldDescriptor::TYPE_BOOL &&
                    b->type() != FieldDescriptor::TYPE_BOOL) {
                    return true;
                }
                
                if (a->type() != FieldDescriptor::TYPE_BOOL &&
                    b->type() == FieldDescriptor::TYPE_BOOL) {
                    return false;
                }
                
                return a->type() < b->type();
            }
        };
        
        struct ExtensionRangeOrdering {
            bool operator()(const Descriptor::ExtensionRange* a,
                            const Descriptor::ExtensionRange* b) const {
                return a->start < b->start;
            }
        };
        
        // Sort the fields of the given Descriptor by number into a new[]'d array
        // and return it.
        const FieldDescriptor** SortFieldsByNumber(const Descriptor* descriptor) {
            const FieldDescriptor** fields = new const FieldDescriptor*[descriptor->field_count()];
            for (int i = 0; i < descriptor->field_count(); i++) {
                fields[i] = descriptor->field(i);
            }
            sort(fields, fields + descriptor->field_count(), FieldOrderingByNumber());
            return fields;
        }
        
        // Sort the fields of the given Descriptor by type into a new[]'d array
        // and return it.
        const FieldDescriptor** SortFieldsByType(const Descriptor* descriptor) {
            const FieldDescriptor** fields = new const FieldDescriptor*[descriptor->field_count()];
            for (int i = 0; i < descriptor->field_count(); i++) {
                fields[i] = descriptor->field(i);
            }
            sort(fields, fields + descriptor->field_count(), FieldOrderingByType());
            return fields;
        }
        
        // Get an identifier that uniquely identifies this type within the file.
        // This is used to declare static variables related to this type at the
        // outermost file scope.
        string UniqueFileScopeIdentifier(const Descriptor* descriptor) {
            return "static_" + StringReplace(descriptor->full_name(), ".", "_", true);
        }
        
        // Returns true if the message type has any required fields.  If it doesn't,
        // we can optimize out calls to its isInitialized() method.
        //
        // already_seen is used to avoid checking the same type multiple times
        // (and also to protect against recursion).
        static bool HasRequiredFields(
                                      const Descriptor* type,
                                      hash_set<const Descriptor*>* already_seen) {
            if (already_seen->count(type) > 0) {
                // The type is already in cache.  This means that either:
                // a. The type has no required fields.
                // b. We are in the midst of checking if the type has required fields,
                //    somewhere up the stack.  In this case, we know that if the type
                //    has any required fields, they'll be found when we return to it,
                //    and the whole call to HasRequiredFields() will return true.
                //    Therefore, we don't have to check if this type has required fields
                //    here.
                return false;
            }
            already_seen->insert(type);
            
            // If the type has extensions, an extension with message type could contain
            // required fields, so we have to be conservative and assume such an
            // extension exists.
            if (type->extension_range_count() > 0) {
                return true;
            }
            
            for (int i = 0; i < type->field_count(); i++) {
                const FieldDescriptor* field = type->field(i);
                if (field->is_required()) {
                    return true;
                }
                if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
                    if (HasRequiredFields(field->message_type(), already_seen)) {
                        return true;
                    }
                }
            }
            
            return false;
        }
        
        static bool HasRequiredFields(const Descriptor* type) {
            hash_set<const Descriptor*> already_seen;
            return HasRequiredFields(type, &already_seen);
        }
    }  // namespace
    
    
    MessageGenerator::MessageGenerator(const Descriptor* descriptor)
    : descriptor_(descriptor),
    field_generators_(descriptor) {
    }
    
    MessageGenerator::MessageGenerator(const Descriptor* descriptor, const string cgiNumber, const string isUpdateFromSvr)
    : descriptor_(descriptor),
    field_generators_(descriptor),  cgiNumber_(cgiNumber), isUpdateFromSvr_(isUpdateFromSvr){
    }
    
    MessageGenerator::~MessageGenerator() {
    }
    
    
    void MessageGenerator::GenerateStaticVariablesHeader(io::Printer* printer) {
        map<string, string> vars;
        vars["identifier"] = UniqueFileScopeIdentifier(descriptor_);
        vars["index"] = SimpleItoa(descriptor_->index());
        vars["classname"] = ClassName(descriptor_);
        if (descriptor_->containing_type() != NULL) {
            vars["parent"] = UniqueFileScopeIdentifier(descriptor_->containing_type());
        }
        
        for (int i = 0; i < descriptor_->nested_type_count(); i++) {
            MessageGenerator(descriptor_->nested_type(i)).GenerateStaticVariablesHeader(printer);
        }
    }
    
    
    void MessageGenerator::GenerateStaticVariablesInitialization(io::Printer* printer) {
        map<string, string> vars;
        vars["identifier"] = UniqueFileScopeIdentifier(descriptor_);
        vars["index"] = SimpleItoa(descriptor_->index());
        vars["classname"] = ClassName(descriptor_);
        if (descriptor_->containing_type() != NULL) {
            vars["parent"] = UniqueFileScopeIdentifier(descriptor_->containing_type());
        }
        
        for (int i = 0; i < descriptor_->extension_count(); i++) {
            ExtensionGenerator(ClassName(descriptor_), descriptor_->extension(i)).GenerateInitializationSource(printer);
        }
        for (int i = 0; i < descriptor_->nested_type_count(); i++) {
            MessageGenerator(descriptor_->nested_type(i)).GenerateStaticVariablesInitialization(printer);
        }
    }
    
    
    void MessageGenerator::GenerateStaticVariablesSource(io::Printer* printer) {
        map<string, string> vars;
        vars["identifier"] = UniqueFileScopeIdentifier(descriptor_);
        vars["index"] = SimpleItoa(descriptor_->index());
        vars["classname"] = ClassName(descriptor_);
        if (descriptor_->containing_type() != NULL) {
            vars["parent"] = UniqueFileScopeIdentifier(descriptor_->containing_type());
        }
        
        for (int i = 0; i < descriptor_->extension_count(); i++) {
            ExtensionGenerator(ClassName(descriptor_), descriptor_->extension(i))
            .GenerateFieldsSource(printer);
        }
        
        for (int i = 0; i < descriptor_->nested_type_count(); i++) {
            MessageGenerator(descriptor_->nested_type(i)).GenerateStaticVariablesSource(printer);
        }
    }
    
    
    void MessageGenerator::DetermineDependencies(set<string>* dependencies) {
        dependencies->insert("@class " + ClassName(descriptor_));
        //dependencies->insert("@class " + ClassName(descriptor_) + "_Builder");
        
        for (int i = 0; i < descriptor_->nested_type_count(); i++) {
            MessageGenerator(descriptor_->nested_type(i)).DetermineDependencies(dependencies);
        }
    }
    
    void MessageGenerator::GenerateEnumHeader(io::Printer* printer) {
        for (int i = 0; i < descriptor_->enum_type_count(); i++) {
            EnumGenerator(descriptor_->enum_type(i)).GenerateHeader(printer);
        }
        
        for (int i = 0; i < descriptor_->nested_type_count(); i++) {
            MessageGenerator(descriptor_->nested_type(i)).GenerateEnumHeader(printer);
        }
    }
    
    void MessageGenerator::GenerateExtensionRegistrationSource(io::Printer* printer) {
        for (int i = 0; i < descriptor_->extension_count(); i++) {
            ExtensionGenerator(ClassName(descriptor_), descriptor_->extension(i))
            .GenerateRegistrationSource(printer);
        }
        
        for (int i = 0; i < descriptor_->nested_type_count(); i++) {
            MessageGenerator(descriptor_->nested_type(i))
            .GenerateExtensionRegistrationSource(printer);
        }
    }
    
    void MessageGenerator::GeneratePropertyIndexMarcoHeader(io::Printer* printer) {
        scoped_array<const FieldDescriptor*> sorted_fields(SortFieldsByType(descriptor_));
        
        for (int i = 0; i < descriptor_->field_count(); i++) {
            string marcoName = ClassName(descriptor_) + "_INDEX_" + descriptor_->field(i)->name();
            std::transform(marcoName.begin(), marcoName.end(), marcoName.begin(), ::toupper);
            printer->Print("\n"
                           "#define $marco_name$",
                           "marco_name", marcoName);
            printer->Print(" $property_index$",
                           "property_index", SimpleItoa(i));
        }
        
        printer->Print("\n");
    }
    
    void MessageGenerator::GenerateMessageHeader(io::Printer* printer) {
        scoped_array<const FieldDescriptor*> sorted_fields(SortFieldsByType(descriptor_));
        
        GeneratePropertyIndexMarcoHeader(printer);
        
        if (descriptor_->extension_range_count() > 0) {
            printer->Print(
                           //        "@interface $classname$ : PBExtendableMessage {\n"
                           //        "@private\n",
                           "\n"
                           "@interface $classname$ : PBExtendableMessage\n\n",
                           "classname", ClassName(descriptor_));
        } else {
            printer->Print(
                           //        "@interface $classname$ : WXPBGeneratedMessage {\n"
                           //        "@private\n",
                           "\n"
                           "@interface $classname$ : WXPBGeneratedMessage\n\n",
                           "classname", ClassName(descriptor_));
        }
        
        //    printer->Indent();
        //    for (int i = 0; i < descriptor_->field_count(); i++) {
        //      field_generators_.get(sorted_fields[i]).GenerateHasFieldHeader(printer);
        //    }
        //    for (int i = 0; i < descriptor_->field_count(); i++) {
        //      field_generators_.get(sorted_fields[i]).GenerateFieldHeader(printer);
        //    }
        //    printer->Outdent();
        
        //    printer->Print("}\n");
        
        /*for (int i = 0; i < descriptor_->field_count(); i++) {
         field_generators_.get(descriptor_->field(i)).GenerateHasPropertyHeader(printer);
         }*/
        for (int i = 0; i < descriptor_->field_count(); i++) {
            field_generators_.get(descriptor_->field(i)).GeneratePropertyHeader(printer);
        }
        printer->Print("\n");
        
        /* yanyang
         for (int i = 0; i < descriptor_->field_count(); i++) {
         field_generators_.get(descriptor_->field(i)).GenerateMembersHeader(printer);
         }
         printer->Print("\n");
         */
        
        //    printer->Print(
        //      "\n",
        //      "fileclass", FileClassName(descriptor_->file()),
        //      "identifier", UniqueFileScopeIdentifier(descriptor_));
        
        for (int i = 0; i < descriptor_->extension_count(); i++) {
            ExtensionGenerator(ClassName(descriptor_), descriptor_->extension(i)).GenerateMembersHeader(printer);
        }
        
        //GenerateIsInitializedHeader(printer);
        //GenerateMessageSerializationMethodsHeader(printer);
        
        GenerateBuilderHeader(printer);
        
        GenerateParseFromMethodsHeader(printer);
        
        printer->Print("@end\n\n");
        
        for (int i = 0; i < descriptor_->nested_type_count(); i++) {
            MessageGenerator(descriptor_->nested_type(i)).GenerateMessageHeader(printer);
        }
        
    }
    
    void MessageGenerator::GenerateMessage(io::Printer *printer) {
        for (int i = 0; i < descriptor_->field_count(); i++) {
            const FieldDescriptor *pFieldDescriptor = descriptor_->field(i);
            if (pFieldDescriptor->is_repeated()) {
                if (pFieldDescriptor->type() == FieldDescriptor::TYPE_MESSAGE) {
                    printer->Print("\"$variblename$\" : [\n ","variblename", UnderscoresToCamelCase(pFieldDescriptor));
                    printer->Indent();
                    printer->Indent();
                    printer->Print("{\n");
                    printer->Indent();
                    printer->Indent();
                    {
                        //printer->Print("$name$","name", ClassName(pFieldDescriptor->message_type()));
                        const Descriptor *pD = pFieldDescriptor->message_type();
                        if (pD == nullptr) {
                            printer->Print("Error occurs: repeated->null message type.");
                            return;
                        }
                        MessageGenerator(pD).GenerateMessage(printer);
                    }
                    printer->Outdent();
                    printer->Outdent();
                    printer->Print("}\n");
                    printer->Outdent();
                    printer->Outdent();
                    if (i == descriptor_->field_count() - 1) {
                        printer->Print("]\n");
                    } else {
                        printer->Print("],\n");
                    }
                } else if (pFieldDescriptor->type() == FieldDescriptor::TYPE_STRING){
                    if (i == descriptor_->field_count() - 1) {
                        printer->Print("\"$variblename$\" : [""]\n","variblename", UnderscoresToCamelCase(pFieldDescriptor));
                    } else {
                        printer->Print("\"$variblename$\" : [""],\n","variblename", UnderscoresToCamelCase(pFieldDescriptor));
                    }
                } else {
                    if (i == descriptor_->field_count() - 1) {
                        printer->Print("\"$variblename$\" : [ ]\n","variblename", UnderscoresToCamelCase(pFieldDescriptor));
                    } else {
                        printer->Print("\"$variblename$\" : [ ],\n","variblename", UnderscoresToCamelCase(pFieldDescriptor));
                    }
                }
            } else {
                ObjectiveCType objcType = GetObjectiveCType(pFieldDescriptor->type());
                if (IsPrimitiveType(objcType)) {
                    if (i == descriptor_->field_count() - 1) {
                        printer->Print("\"$variblename$\" : 0\n","variblename", UnderscoresToCamelCase(pFieldDescriptor));
                    } else {
                        printer->Print("\"$variblename$\" : 0,\n","variblename", UnderscoresToCamelCase(pFieldDescriptor));
                    }
                } else if (pFieldDescriptor->type() == FieldDescriptor::TYPE_STRING || pFieldDescriptor->type() == FieldDescriptor::TYPE_BYTES || pFieldDescriptor->message_type()->name() == "SKBuiltinString_t") {
                    if (i == descriptor_->field_count() - 1) {
                        printer->Print("\"$variblename$\" : \"\"\n","variblename", UnderscoresToCamelCase(pFieldDescriptor));
                    } else {
                        printer->Print("\"$variblename$\" : \"\" ,\n","variblename", UnderscoresToCamelCase(pFieldDescriptor));
                    }
                } else if (pFieldDescriptor->message_type()->name() == "SKBuiltinBuffer_t") {
                    if (i == descriptor_->field_count() - 1) {
                        printer->Print("\"$variblename$\" : {}\n","variblename", UnderscoresToCamelCase(pFieldDescriptor));
                    } else {
                        printer->Print("\"$variblename$\" : {},\n","variblename", UnderscoresToCamelCase(pFieldDescriptor));
                    }
                }
                else if (pFieldDescriptor->type() == FieldDescriptor::TYPE_MESSAGE) {
                    const Descriptor *pD = pFieldDescriptor->message_type();
                    if (pD == nullptr) {
                        printer->Print("Error occurs: not repeated->null message type.");
                        return;
                    }
                    printer->Print("\"$variblename$\" : {\n","variblename", UnderscoresToCamelCase(pFieldDescriptor));
                    printer->Indent();
                    printer->Indent();
                    MessageGenerator(pD).GenerateMessage(printer);
                    printer->Outdent();
                    printer->Outdent();
                    if (i == descriptor_->field_count() - 1) {
                        printer->Print("}\n");
                    } else {
                        printer->Print("},\n");
                    }
                }
            }
        }
    }
    
    void MessageGenerator::GenerateMockCase(io::Printer *printer) {
        //printer->Print("var response = {\n");
        //descriptor_->file_
        printer->Print("{\n");
        printer->Indent();
        printer->Indent();
        GenerateMessage(printer);
        printer->Outdent();
        printer->Outdent();
        printer->Print("}\n\n");
        map<string,string> vars;
        vars["cgiNumber"] = cgiNumber_;
        vars["isUpdateFromSvr"] = isUpdateFromSvr_;
        //string s = string(printer->buffer_);
        printf("%s", printer->s_.c_str());
        //printer->Print(vars, "mockRequest($cgiNumber$).isUpdateFromSvr($isUpdateFromSvr$).withResponse(response)");
        
    }
    
    void MessageGenerator::GenerateSource(io::Printer* printer) {
        /* yanyang
         printer->Print(
         "\n"
         "@interface $classname$ ()\n",
         "classname", ClassName(descriptor_));
         for (int i = 0; i < descriptor_->field_count(); i++) {
         field_generators_.get(descriptor_->field(i)).GenerateExtensionSource(printer);
         }
         printer->Print("@end\n\n");
         */
        
        //    printer->Print("@implementation $classname$\n\n",
        printer->Print("@implementation $classname$ {\n",
                       "classname", ClassName(descriptor_));
        
        printer->Indent();
        for (int i = 0; i < descriptor_->field_count(); i++) {
            field_generators_.get(descriptor_->field(i)).GenerateImplementationDeclarationSource(printer);
        }
        printer->Outdent();
        printer->Print("}\n\n");
        
        for (int i = 0; i < descriptor_->field_count(); i++) {
            field_generators_.get(descriptor_->field(i)).GenerateSynthesizeSource(printer);
        }
        printer->Print("\n");
        
        //printer->Print("- (void) dealloc {\n");
        //printer->Indent();
        //for (int i = 0; i < descriptor_->field_count(); i++) {
        //  field_generators_.get(descriptor_->field(i)).GenerateDeallocSource(printer);
        //}
        //printer->Outdent();
        //printer->Print("}\n");
        
        /* yanyang
        printer->Print(
                       "- (id) init {\n"
                       "  if ((self = [super init])) {\n");
        printer->Indent();
        printer->Indent();
        for (int i = 0; i < descriptor_->field_count(); i++) {
            field_generators_.get(descriptor_->field(i)).GenerateInitializationSource(printer);
        }
        printer->Outdent();
        printer->Outdent();
        printer->Print(
                       "    [self clearHasBits];\n"
                       "  }\n"
                       "  return self;\n"
                       "}\n");
         */
        
        for (int i = 0; i < descriptor_->extension_count(); i++) {
            ExtensionGenerator(ClassName(descriptor_), descriptor_->extension(i)).GenerateMembersSource(printer);
        }
        
        for (int i = 0; i < descriptor_->field_count(); i++) {
            field_generators_.get(descriptor_->field(i)).GenerateMembersSource(printer);
        }
        
        GenerateIsInitializedSource(printer);
        GenerateMessageSerializationMethodsSource(printer);
        
        GenerateParseFromMethodsSource(printer);
        
        /*printer->Print(
         "+ ($classname$_Builder*) builder {\n"
         "  return [[$classname$_Builder alloc] init];\n"
         "}\n",
         "classname", ClassName(descriptor_));*/
        
        GenerateBuilderSource(printer);
        
        printer->Print("@end\n\n");
        
        for (int i = 0; i < descriptor_->enum_type_count(); i++) {
            EnumGenerator(descriptor_->enum_type(i)).GenerateSource(printer);
        }
        
        for (int i = 0; i < descriptor_->nested_type_count(); i++) {
            MessageGenerator(descriptor_->nested_type(i)).GenerateSource(printer);
        }
    }
    
    
    void MessageGenerator::GenerateMessageSerializationMethodsHeader(io::Printer* printer) {
        scoped_array<const FieldDescriptor*> sorted_fields(SortFieldsByNumber(descriptor_));
        
        vector<const Descriptor::ExtensionRange*> sorted_extensions;
        for (int i = 0; i < descriptor_->extension_range_count(); ++i) {
            sorted_extensions.push_back(descriptor_->extension_range(i));
        }
        sort(sorted_extensions.begin(), sorted_extensions.end(),
             ExtensionRangeOrdering());
    }
    
    void MessageGenerator::GenerateParseFromMethodsHeader(io::Printer* printer) {
        /* yanyang
        printer->Print(
                       "+ ($classname$*) parseFromData:(NSData*) data;\n",
                       "classname", ClassName(descriptor_));
         */
    }
    
    
    void MessageGenerator::GenerateSerializeOneFieldHeader(
                                                           io::Printer* printer, const FieldDescriptor* field) {
        field_generators_.get(field).GenerateSerializationCodeHeader(printer);
    }
    
    
    void MessageGenerator::GenerateSerializeOneExtensionRangeHeader(
                                                                    io::Printer* printer, const Descriptor::ExtensionRange* range) {
    }
    
    
    void MessageGenerator::GenerateBuilderHeader(io::Printer* printer) {
        /*{
         printer->Print(
         "@interface $classname$_Builder : WXPBGeneratedMessage_Builder {\n"
         "@private\n"
         "  $classname$* result;\n"
         "}\n",
         "classname", ClassName(descriptor_));
         }*/
        
        //GenerateCommonBuilderMethodsHeader(printer);
        //GenerateBuilderParsingMethodsHeadea(printer);
        
        /* yanyang
         for (int i = 0; i < descriptor_->field_count(); i++) {
         field_generators_.get(descriptor_->field(i)).GenerateBuilderMembersHeader(printer);
         }
         */
        
        //printer->Print("@end\n\n");
    }
    
    
    void MessageGenerator::GenerateCommonBuilderMethodsHeader(io::Printer* printer) {
        /*printer->Print(
         "\n"
         "- ($classname$*) build;\n",
         "classname", ClassName(descriptor_));*/
        printer->Indent();
        
        for (int i = 0; i < descriptor_->field_count(); i++) {
            field_generators_.get(descriptor_->field(i)).GenerateBuildingCodeHeader(printer);
        }
        
        printer->Outdent();
        
        printer->Indent();
        
        for (int i = 0; i < descriptor_->field_count(); i++) {
            field_generators_.get(descriptor_->field(i)).GenerateMergingCodeHeader(printer);
        }
        
        printer->Outdent();
    }
    
    
    void MessageGenerator::GenerateBuilderParsingMethodsHeader(io::Printer* printer) {
        scoped_array<const FieldDescriptor*> sorted_fields(SortFieldsByNumber(descriptor_));
    }
    
    
    /*void MessageGenerator::GenerateIsInitializedHeader(io::Printer* printer) {
     printer->Print(
     "- (BOOL) isInitialized;\n");
     }*/
    
    
    void MessageGenerator::GenerateMessageSerializationMethodsSource(io::Printer* printer) {
        // yanyang
        return;
        
        scoped_array<const FieldDescriptor*> sorted_fields(SortFieldsByNumber(descriptor_));
        
        vector<const Descriptor::ExtensionRange*> sorted_extensions;
        for (int i = 0; i < descriptor_->extension_range_count(); ++i) {
            sorted_extensions.push_back(descriptor_->extension_range(i));
        }
        sort(sorted_extensions.begin(), sorted_extensions.end(),
             ExtensionRangeOrdering());
        
        printer->Print(
                       "- (void) writeToCodedOutputStream:(PBCodedOutputStream*) output {\n");
        printer->Indent();
        
        // Merge the fields and the extension ranges, both sorted by field number.
        for (int i = 0, j = 0;
             i < descriptor_->field_count() || j < sorted_extensions.size(); ) {
            if (i == descriptor_->field_count()) {
                GenerateSerializeOneExtensionRangeSource(printer, sorted_extensions[j++]);
            } else if (j == sorted_extensions.size()) {
                GenerateSerializeOneFieldSource(printer, sorted_fields[i++]);
            } else if (sorted_fields[i]->number() < sorted_extensions[j]->start) {
                GenerateSerializeOneFieldSource(printer, sorted_fields[i++]);
            } else {
                GenerateSerializeOneExtensionRangeSource(printer, sorted_extensions[j++]);
            }
        }
        
        /*if (descriptor_->options().message_set_wire_format()) {
         printer->Print(
         "[self.unknownFields writeAsMessageSetTo:output];\n");
         } else {
         printer->Print(
         "[self.unknownFields writeToCodedOutputStream:output];\n");
         }*/
        
        printer->Outdent();
        printer->Print(
                       "}\n"
                       "- (int32_t) serializedSize {\n"
                       "  int32_t iSize = 0;\n");
        printer->Indent();
        
        for (int i = 0; i < descriptor_->field_count(); i++) {
            field_generators_.get(sorted_fields[i]).GenerateSerializedSizeCodeSource(printer);
        }
        
        if (descriptor_->extension_range_count() > 0) {
            printer->Print(
                           "iSize += [self extensionsSerializedSize];\n");
        }
        
        /*if (descriptor_->options().message_set_wire_format()) {
         printer->Print(
         "iSize += self.unknownFields.serializedSizeAsMessageSet;\n");
         } else {
         printer->Print(
         "iSize += self.unknownFields.serializedSize;\n");
         }*/
        
        printer->Outdent();
        printer->Print(
                       "  return iSize;\n"
                       "}\n");
    }
    
    void MessageGenerator::GenerateParseFromMethodsSource(io::Printer* printer) {
        /* yanyang
        printer->Print(
                       "+ ($classname$*) parseFromData:(NSData*) data {\n"
                       "  return ($classname$*)[[[$classname$ alloc] init] mergeFromData:data];\n"
                       "}\n",
                       "classname", ClassName(descriptor_));
         */
    }
    
    
    void MessageGenerator::GenerateSerializeOneFieldSource(
                                                           io::Printer* printer, const FieldDescriptor* field) {
        field_generators_.get(field).GenerateSerializationCodeSource(printer);
    }
    
    
    void MessageGenerator::GenerateSerializeOneExtensionRangeSource(
                                                                    io::Printer* printer, const Descriptor::ExtensionRange* range) {
        printer->Print(
                       "[self writeExtensionsToCodedOutputStream:output\n"
                       "                                    from:$from$\n"
                       "                                      to:$to$];\n",
                       "from", SimpleItoa(range->start),
                       "to", SimpleItoa(range->end));
    }
    
    
    void MessageGenerator::GenerateBuilderSource(io::Printer* printer) {
        /*printer->Print(
         "@interface $classname$_Builder()\n"
         "@property (strong) $classname$* result;\n"
         "@end\n"
         "\n"
         "@implementation $classname$_Builder\n"
         "@synthesize result;\n"
         //"- (void) dealloc {\n"
         //"  self.result = nil;\n"
         //"}\n",
         "classname", ClassName(descriptor_));
         
         printer->Print(
         "- (id) init {\n"
         "  if ((self = [super init])) {\n"
         "    self.result = [[$classname$ alloc] init];\n"
         "  }\n"
         "  return self;\n"
         "}\n",
         "classname", ClassName(descriptor_));*/
        
        //GenerateCommonBuilderMethodsSource(printer);
        GenerateBuilderParsingMethodsSource(printer);
        GenerateInitializeSource(printer);
        
        /* yanyang
         for (int i = 0; i < descriptor_->field_count(); i++) {
         field_generators_.get(descriptor_->field(i)).GenerateBuilderMembersSource(printer);
         }
         */
        
        //printer->Print("@end\n\n");
        
        //littleliang
        //descriptor_
        bool isNeedAddChangeModel = false;
        for (int i = 0; i < descriptor_->field_count(); i++) {
            const FieldDescriptor  *d = descriptor_->field(i);
            if (d->type() == FieldDescriptor::TYPE_MESSAGE && d->is_repeated()) {
                isNeedAddChangeModel = true;
                break;
            }
        }
        if (isNeedAddChangeModel) {
            GenerateModelMapping(printer);
        }
        
    }
    
    
    void MessageGenerator::GenerateCommonBuilderMethodsSource(io::Printer* printer) {
        printer->Print(
                       "- ($classname$*) build {\n"
                       "  //[self checkInitialized];\n",
                       "classname", ClassName(descriptor_));
        printer->Indent();
        
        for (int i = 0; i < descriptor_->field_count(); i++) {
            field_generators_.get(descriptor_->field(i)).GenerateBuildingCodeSource(printer);
        }
        
        printer->Outdent();
        printer->Print(
                       "  $classname$* returnMe = result;\n"
                       "  self.result = nil;\n"
                       "  return returnMe;\n"
                       "}\n",
                       "classname", ClassName(descriptor_));
    }
    
    
    void MessageGenerator::GenerateBuilderParsingMethodsSource(io::Printer* printer) {
        // yanyang
        return;
        
        scoped_array<const FieldDescriptor*> sorted_fields(
                                                           SortFieldsByNumber(descriptor_));
        
        printer->Print(
                       "- ($classname$*) mergeFromCodedInputStream:(PBCodedInputStream*) input {\n",
                       "classname", ClassName(descriptor_));
        printer->Indent();
        
        printer->Print("while (YES) {\n");
        printer->Indent();
        
        printer->Print(
                       "int32_t tag = [input readTag];\n"
                       "switch (tag) {\n");
        printer->Indent();
        
        printer->Print(
                       "case 0:\n"          // zero signals EOF / limit reached
                       "  return self;\n"
                       "default: {\n"
                       "  if (![input skipField:tag]) {\n"
                       "    return self;\n"   // it's an endgroup tag
                       "  }\n"
                       "  break;\n"
                       "}\n");
        
        for (int i = 0; i < descriptor_->field_count(); i++) {
            const FieldDescriptor* field = sorted_fields[i];
            uint32 tag = WireFormatLite::MakeTag(field->number(),
                                                 WireFormat::WireTypeForField(field));
            
            printer->Print(
                           "case $tag$: {\n",
                           "tag", SimpleItoa(tag));
            printer->Indent();
            
            field_generators_.get(field).GenerateParsingCodeSource(printer);
            
            printer->Outdent();
            printer->Print(
                           "  break;\n"
                           "}\n");
        }
        
        printer->Outdent();
        printer->Outdent();
        printer->Outdent();
        printer->Print(
                       "    }\n" // switch (tag)
                       "  }\n"   // while (true)
                       "}\n");
    }
    
    void MessageGenerator::GenerateInitializeSource(io::Printer* printer) {
        
        printer->Print(
                       "+ (void) initialize {\n");
        printer->Indent();
        printer->Print(
                       "static PBFieldInfo _fieldInfoArray[] = {\n");
        
        printer->Indent();
        for (int i = 0; i < descriptor_->field_count(); i++) {
            field_generators_.get(descriptor_->field(i)).GenerateInitializeCodeSource(printer);
        }
        printer->Outdent();
        
        printer->Print(
                       "};\n"
                       "initializePBClassInfo(self, _fieldInfoArray);\n");
        
        printer->Outdent();
        printer->Print(
                       "}\n\n");
    }
    
    void MessageGenerator::GenerateModelMapping(io::Printer* printer) {
        printer->Print("#ifdef CGI_MOCK\n");
        printer->Print(
                       "+ (NSDictionary *) modelContainerPropertyMapping {\n");
        printer->Indent();
        printer->Print("return @{\n");
        printer->Indent();
        for (int i = 0; i < descriptor_->field_count(); i++) {
            const FieldDescriptor  *field = descriptor_->field(i);
            if (field->type() == FieldDescriptor::TYPE_MESSAGE && field->is_repeated()) {
                map<string,string> vars;
                vars["type"] = ClassName(field->message_type());
                vars["name"] = UnderscoresToCamelCase(field);
                vars["capitalized_name"] = UnderscoresToCapitalizedCamelCase(field);
                vars["index"] = SimpleItoa(field->index());
                printer->Print(vars, "@\"$name$\":@\"$type$\",\n");
            }
        }
        printer->Outdent();
        printer->Print("};\n");
        printer->Outdent();
        printer->Print("}\n");
        printer->Print("#endif\n\n");
    }
    /*
    enum Type {
        TYPE_DOUBLE         = 1,   // double, exactly eight bytes on the wire.
        TYPE_FLOAT          = 2,   // float, exactly four bytes on the wire.
        TYPE_INT64          = 3,   // int64, varint on the wire.  Negative numbers
        // take 10 bytes.  Use TYPE_SINT64 if negative
        // values are likely.
        TYPE_UINT64         = 4,   // uint64, varint on the wire.
        TYPE_INT32          = 5,   // int32, varint on the wire.  Negative numbers
        // take 10 bytes.  Use TYPE_SINT32 if negative
        // values are likely.
        TYPE_FIXED64        = 6,   // uint64, exactly eight bytes on the wire.
        TYPE_FIXED32        = 7,   // uint32, exactly four bytes on the wire.
        TYPE_BOOL           = 8,   // bool, varint on the wire.
        TYPE_STRING         = 9,   // UTF-8 text.
        TYPE_GROUP          = 10,  // Tag-delimited message.  Deprecated.
        TYPE_MESSAGE        = 11,  // Length-delimited message.
        
        TYPE_BYTES          = 12,  // Arbitrary byte array.
        TYPE_UINT32         = 13,  // uint32, varint on the wire
        TYPE_ENUM           = 14,  // Enum, varint on the wire
        TYPE_SFIXED32       = 15,  // int32, exactly four bytes on the wire
        TYPE_SFIXED64       = 16,  // int64, exactly eight bytes on the wire
        TYPE_SINT32         = 17,  // int32, ZigZag-encoded varint on the wire
        TYPE_SINT64         = 18,  // int64, ZigZag-encoded varint on the wire
        
        MAX_TYPE            = 18,  // Constant useful for defining lookup tables
        // indexed by Type.
    };
     */
    
//    bool MessageGenerator::IsPrimitiveType(FieldDescriptor::Type type) {
//        if (type == FieldDescriptor::TYPE_DOUBLE || type == FieldDescriptor::TYPE_FLOAT || type == FieldDescriptor::TYPE_INT64 || type == FieldDescriptor::TYPE_UINT64 || type == FieldDescriptor::TYPE_INT32 || type == FieldDescriptor::TYPE_FIXED64 || type == FieldDescriptor::TYPE_FIXED32 || type == FieldDescriptor::TYPE_FIXED32 || type == FieldDescriptor::TYPE_BOOL || type == FieldDescriptor::TYPE_UINT32 || type == FieldDescriptor::TYPE_ENUM || type == FieldDescriptor::TYPE_SFIXED32 || type == FieldDescriptor::TYPE_SFIXED64 || type == FieldDescriptor::TYPE_SINT32 || type == FieldDescriptor::TYPE_SINT64) {
//            return true;
//        }
//        return false;
//    }
    
    void MessageGenerator::GenerateIsInitializedSource(io::Printer* printer) {
        // yanyang
        return;

        printer->Print(
                       "- (BOOL) isInitialized {\n");
        printer->Indent();
        
        // Check that all required fields in this message are set.
        // TODO(kenton):  We can optimize this when we switch to putting all the
        //   "has" fields into a single bitfield.
        for (int i = 0; i < descriptor_->field_count(); i++) {
            const FieldDescriptor* field = descriptor_->field(i);
            
            if (field->is_required()) {
                /* yanyang
                printer->Print(
                               "if (!self.has$capitalized_name$) {\n"
                               "  return NO;\n"
                               "}\n",
                               "capitalized_name", UnderscoresToCapitalizedCamelCase(field));
                 */
                printer->Print(
                               "if (!_HAS_BIT($index$)) {\n"
                               "  return NO;\n"
                               "}\n",
                               "index", SimpleItoa(field->index()));

            }
        }
        
        // Now check that all embedded messages are initialized.
        for (int i = 0; i < descriptor_->field_count(); i++) {
            const FieldDescriptor* field = descriptor_->field(i);
            if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE &&
                HasRequiredFields(field->message_type())) {
                
                map<string,string> vars;
                vars["type"] = ClassName(field->message_type());
                vars["name"] = UnderscoresToCamelCase(field);
                vars["capitalized_name"] = UnderscoresToCapitalizedCamelCase(field);
                vars["index"] = SimpleItoa(field->index());
                
                switch (field->label()) {
                    case FieldDescriptor::LABEL_REQUIRED:
                        printer->Print(vars,
                                       "if (!self.$name$.isInitialized) {\n"
                                       "  return NO;\n"
                                       "}\n");
                        break;
                    case FieldDescriptor::LABEL_OPTIONAL:
                        /* yanyang
                        printer->Print(vars,
                                       "if (self.has$capitalized_name$) {\n"
                                       "  if (!self.$name$.isInitialized) {\n"
                                       "    return NO;\n"
                                       "  }\n"
                                       "}\n");
                         */
                        printer->Print(vars,
                                       "if (_HAS_BIT($index$)) {\n"
                                       "  if (!self.$name$.isInitialized) {\n"
                                       "    return NO;\n"
                                       "  }\n"
                                       "}\n");
                        break;
                    case FieldDescriptor::LABEL_REPEATED:
                        printer->Print(vars,
                                       "for ($type$* element in self.$name$) {\n"
                                       "  if (!element.isInitialized) {\n"
                                       "    return NO;\n"
                                       "  }\n"
                                       "}\n");
                        break;
                }
            }
        }
        
        if (descriptor_->extension_range_count() > 0) {
            printer->Print(
                           "if (!self.extensionsAreInitialized) {\n"
                           "  return NO;\n"
                           "}\n");
        }
        
        printer->Outdent();
        printer->Print(
                       "  return YES;\n"
                       "}\n");
    }
}  // namespace objectivec
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
