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

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include <map>
#include <string>

#include <google/protobuf/compiler/objectivec/objectivec_primitive_field.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/compiler/objectivec/objectivec_helpers.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/wire_format.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/stubs/substitute.h>

namespace google { namespace protobuf { namespace compiler { namespace objectivec {
    
    using internal::WireFormat;
    using internal::WireFormatLite;
    
    namespace {
        
        const char* PrimitiveTypeName(ObjectiveCType type) {
            switch (type) {
                case OBJECTIVECTYPE_INT    : return "int32_t";
                case OBJECTIVECTYPE_UINT   : return "uint32_t";
                case OBJECTIVECTYPE_LONG   : return "int64_t";
                case OBJECTIVECTYPE_ULONG  : return "uint64_t";
                case OBJECTIVECTYPE_FLOAT  : return "Float32";
                case OBJECTIVECTYPE_DOUBLE : return "Float64";
                case OBJECTIVECTYPE_BOOLEAN: return "BOOL";
                case OBJECTIVECTYPE_STRING : return "NSString";
                case OBJECTIVECTYPE_DATA   : return "NSData";
                case OBJECTIVECTYPE_ENUM   : return "int32_t";
                case OBJECTIVECTYPE_MESSAGE: return NULL;
            }
            
            GOOGLE_LOG(FATAL) << "Can't get here.";
            return NULL;
        }
        
        const char* GetCapitalizedType(const FieldDescriptor* field) {
            switch (field->type()) {
                case FieldDescriptor::TYPE_INT32   : return "Int32"   ;
                case FieldDescriptor::TYPE_UINT32  : return "UInt32"  ;
                case FieldDescriptor::TYPE_SINT32  : return "SInt32"  ;
                case FieldDescriptor::TYPE_FIXED32 : return "Fixed32" ;
                case FieldDescriptor::TYPE_SFIXED32: return "SFixed32";
                case FieldDescriptor::TYPE_INT64   : return "Int64"   ;
                case FieldDescriptor::TYPE_UINT64  : return "UInt64"  ;
                case FieldDescriptor::TYPE_SINT64  : return "SInt64"  ;
                case FieldDescriptor::TYPE_FIXED64 : return "Fixed64" ;
                case FieldDescriptor::TYPE_SFIXED64: return "SFixed64";
                case FieldDescriptor::TYPE_FLOAT   : return "Float"   ;
                case FieldDescriptor::TYPE_DOUBLE  : return "Double"  ;
                case FieldDescriptor::TYPE_BOOL    : return "Bool"    ;
                case FieldDescriptor::TYPE_STRING  : return "String"  ;
                case FieldDescriptor::TYPE_BYTES   : return "Data"   ;
                case FieldDescriptor::TYPE_ENUM    : return "Enum"    ;
                case FieldDescriptor::TYPE_GROUP   : return "Group"   ;
                case FieldDescriptor::TYPE_MESSAGE : return "Message" ;
            }
            
            GOOGLE_LOG(FATAL) << "Can't get here.";
            return NULL;
        }
        
        // For encodings with fixed sizes, returns that size in bytes.  Otherwise
        // returns -1.
        int FixedSize(FieldDescriptor::Type type) {
            switch (type) {
                case FieldDescriptor::TYPE_INT32   : return -1;
                case FieldDescriptor::TYPE_INT64   : return -1;
                case FieldDescriptor::TYPE_UINT32  : return -1;
                case FieldDescriptor::TYPE_UINT64  : return -1;
                case FieldDescriptor::TYPE_SINT32  : return -1;
                case FieldDescriptor::TYPE_SINT64  : return -1;
                case FieldDescriptor::TYPE_FIXED32 : return WireFormatLite::kFixed32Size;
                case FieldDescriptor::TYPE_FIXED64 : return WireFormatLite::kFixed64Size;
                case FieldDescriptor::TYPE_SFIXED32: return WireFormatLite::kSFixed32Size;
                case FieldDescriptor::TYPE_SFIXED64: return WireFormatLite::kSFixed64Size;
                case FieldDescriptor::TYPE_FLOAT   : return WireFormatLite::kFloatSize;
                case FieldDescriptor::TYPE_DOUBLE  : return WireFormatLite::kDoubleSize;
                    
                case FieldDescriptor::TYPE_BOOL    : return WireFormatLite::kBoolSize;
                case FieldDescriptor::TYPE_ENUM    : return -1;
                    
                case FieldDescriptor::TYPE_STRING  : return -1;
                case FieldDescriptor::TYPE_BYTES   : return -1;
                case FieldDescriptor::TYPE_GROUP   : return -1;
                case FieldDescriptor::TYPE_MESSAGE : return -1;
                    
                    // No default because we want the compiler to complain if any new
                    // types are added.
            }
            GOOGLE_LOG(FATAL) << "Can't get here.";
            return -1;
        }
        
        
        void SetPrimitiveVariables(const FieldDescriptor* descriptor,
                                   map<string, string>* variables) {
            
            const string& descriptorCamelCase = UnderscoresToCamelCase(descriptor);
            const string& descriptorCapitalizedCamelCase = UnderscoresToCapitalizedCamelCase(descriptor);
            const ObjectiveCType& objc_type = GetObjectiveCType(descriptor);
            
            (*variables)["classname"] = ClassName(descriptor->containing_type());
            (*variables)["name"] = descriptorCamelCase;
            (*variables)["capitalized_name"] = descriptorCapitalizedCamelCase;
            (*variables)["list_name"] = descriptorCamelCase + "List";
            (*variables)["mutable_list_name"] = "mutable" + descriptorCapitalizedCamelCase + "List";
            (*variables)["number"] = SimpleItoa(descriptor->number());
            (*variables)["type"] = PrimitiveTypeName(objc_type);
            
            if (IsPrimitiveType(objc_type)) {
                (*variables)["storage_type"] = PrimitiveTypeName(objc_type);
            } else {
                (*variables)["storage_type"] = string(PrimitiveTypeName(objc_type)) + "*";
            }
            
            (*variables)["boxed_type"] = BoxedPrimitiveTypeName(objc_type);
            (*variables)["default"] = DefaultValue(descriptor);
            (*variables)["capitalized_type"] = GetCapitalizedType(descriptor);
            (*variables)["boxed_value"] = BoxValue(descriptor, "value");
            
            string unboxed_value = "value";
            switch (objc_type) {
                case OBJECTIVECTYPE_INT:
                    unboxed_value = "[value intValue]";
                    break;
                case OBJECTIVECTYPE_UINT:
                    unboxed_value = "[value unsignedIntValue]";
                    break;
                case OBJECTIVECTYPE_LONG:
                    unboxed_value = "[value longLongValue]";
                    break;
                case OBJECTIVECTYPE_ULONG:
                    unboxed_value = "[value unsignedLongLongValue]";
                    break;
                case OBJECTIVECTYPE_FLOAT:
                    unboxed_value = "[value floatValue]";
                    break;
                case OBJECTIVECTYPE_DOUBLE:
                    unboxed_value = "[value doubleValue]";
                    break;
                case OBJECTIVECTYPE_BOOLEAN:
                    unboxed_value = "[value boolValue]";
                    break;
            }
            
            (*variables)["unboxed_value"] = unboxed_value;
            
            (*variables)["tag"] = SimpleItoa(WireFormat::MakeTag(descriptor));
            (*variables)["tag_size"] = SimpleItoa(
                                                  WireFormat::TagSize(descriptor->number(), descriptor->type()));
            
            int fixed_size = FixedSize(descriptor->type());
            if (fixed_size != -1) {
                (*variables)["fixed_size"] = SimpleItoa(fixed_size);
            }
            
            (*variables)["member_name"] = descriptorCamelCase;
            if (descriptor->type() ==  FieldDescriptor::TYPE_BOOL
                || IsObjectiveCKeyWord(descriptorCamelCase)
                || IsComplieKeyWord(descriptorCamelCase)) {
                (*variables)["member_name"] = descriptorCamelCase + "_";
            }
            
            (*variables)["list_name_method_attribute"] = "";
            if (HasOwnedObjectPrefix((*variables)["list_name"])) {
                (*variables)["list_name_method_attribute"] = " PB_OBJC_METHOD_FAMILY_NONE";
            }
            
            // yanyang
            (*variables)["index"] = SimpleItoa(descriptor->index());
            (*variables)["field_label"] = FieldLabelName(descriptor);
            (*variables)["field_type"] = FieldTypeName(descriptor);
            (*variables)["is_packed"] = (descriptor->options().packed() ? "YES" : "NO");
        }
    }  // namespace
    
    
    PrimitiveFieldGenerator::PrimitiveFieldGenerator(const FieldDescriptor* descriptor)
    : descriptor_(descriptor) {
        SetPrimitiveVariables(descriptor, &variables_);
    }
    
    
    PrimitiveFieldGenerator::~PrimitiveFieldGenerator() {
    }
    
    
    void PrimitiveFieldGenerator::GenerateHasFieldHeader(io::Printer* printer) const {
        //    printer->Print(variables_, "BOOL has$capitalized_name$:1;\n");
    }
    
    
    void PrimitiveFieldGenerator::GenerateFieldHeader(io::Printer* printer) const {
        //    if (descriptor_->type() ==  FieldDescriptor::TYPE_BOOL) {
        //      printer->Print(variables_, "$storage_type$ $name$_:1;\n");
        //    } else {
        //      printer->Print(variables_, "$storage_type$ $name$;\n");
        //    }
    }
    
    
    void PrimitiveFieldGenerator::GenerateHasPropertyHeader(io::Printer* printer) const {
        //    printer->Print(variables_, "- (BOOL) has$capitalized_name$;\n");
    }
    
    void PrimitiveFieldGenerator::GeneratePropertyHeader(io::Printer* printer) const {
        //    if (IsReferenceType(GetObjectiveCType(descriptor_))) {
        //      printer->Print(variables_,
        //        "@property (readonly, strong) $storage_type$ $name$;\n");
        //    } else if (GetObjectiveCType(descriptor_) == OBJECTIVECTYPE_BOOLEAN) {
        //      printer->Print(variables_,
        //        "- (BOOL) $name$;\n");
        //    } else {
        //      printer->Print(variables_,
        //        "@property (readonly) $storage_type$ $name$;\n");
        //    }
        
        /* yanyang
        printer->Print(variables_, "@property(nonatomic, assign, readonly) BOOL has$capitalized_name$;\n");
        
        if (IsReferenceType(GetObjectiveCType(descriptor_))) {
            printer->Print(variables_,
                           "@property (nonatomic, strong, setter=Set$capitalized_name$:) $storage_type$ $name$;\n");
        } else {
            printer->Print(variables_,
                           "@property (nonatomic, assign, setter=Set$capitalized_name$:) $storage_type$ $name$;\n");
        }
         */
        if (IsReferenceType(GetObjectiveCType(descriptor_))) {
            printer->Print(variables_,
                           "@property (nonatomic, strong) $storage_type$ $name$;\n");
        } else {
            printer->Print(variables_,
                           "@property (nonatomic, assign) $storage_type$ $name$;\n");
        }
        
        if (HasOwnedObjectPrefixInField(variables_, "name")) {
            printer->Print(variables_,
                           "- ($storage_type$)$name$ PB_OBJC_METHOD_FAMILY_NONE;\n");
        }
    }
    
    
    void PrimitiveFieldGenerator::GenerateExtensionSource(io::Printer* printer) const {
        //    if (IsReferenceType(GetObjectiveCType(descriptor_))) {
        //      printer->Print(variables_,
        //	    "@property BOOL has$capitalized_name$;\n"
        //        "@property (strong) $storage_type$ $name$;\n");
        //    } else {
        //      printer->Print(variables_,
        //	    "@property BOOL has$capitalized_name$;\n"
        //        "@property $storage_type$ $name$;\n");
        //    }
    }
    
    
    void PrimitiveFieldGenerator::GenerateImplementationDeclarationSource(io::Printer* printer) const {
        // yanyang
        //printer->Print(variables_, "BOOL has$capitalized_name$:1;\n");
        
        /*
        if (descriptor_->type() ==  FieldDescriptor::TYPE_BOOL) {
            printer->Print(variables_, "$storage_type$ $member_name$:1;\n");
        }
        else if (AreFieldsDifferent(variables_, "name", "member_name")) {
            printer->Print(variables_, "$storage_type$ $member_name$;\n");
        }
         */
    }
    
    
    void PrimitiveFieldGenerator::GenerateSynthesizeSource(io::Printer* printer) const {
        // yanyang
        //printer->Print(variables_, "@synthesize has$capitalized_name$;\n");
        
        /*if (AreFieldsDifferent(variables_, "name", "member_name")) {
            printer->Print(variables_, "@synthesize $name$ = $member_name$;\n");
        }
        else {*/
            printer->Print(variables_, "PB_PROPERTY_TYPE $name$;\n");
        //}
    }
    
    
    void PrimitiveFieldGenerator::GenerateDeallocSource(io::Printer* printer) const {
        if (IsReferenceType(GetObjectiveCType(descriptor_))) {
            printer->Print(variables_,
                           "self.$name$ = nil;\n");
        }
    }
    
    
    void PrimitiveFieldGenerator::GenerateInitializationSource(io::Printer* printer) const {
        /* yanyang
        printer->Print(variables_,
                       "self.$name$ = $default$;\n"
                       "has$capitalized_name$ = NO;\n");
         */
    }
    
    
    void PrimitiveFieldGenerator::GenerateMembersHeader(io::Printer* printer) const {
    }
    
    
    void PrimitiveFieldGenerator::GenerateMembersSource(io::Printer* printer) const {
        /* yanyang
        if (GetObjectiveCType(descriptor_) == OBJECTIVECTYPE_BOOLEAN) {
            printer->Print(variables_,
                           "- (BOOL) $name$ {\n"
                           "  return !!$member_name$;\n"
                           "}\n"
                           "- (void) Set$capitalized_name$:(BOOL) value {\n"
                           "  has$capitalized_name$ = YES;\n"
                           "  $member_name$ = !!value;\n"
                           "}\n");
        }
        else if (IsReferenceType(GetObjectiveCType(descriptor_))) {
            printer->Print(variables_,
                           "- (void) Set$capitalized_name$:($storage_type$) value {\n"
                           "  has$capitalized_name$ = YES;\n"
                           "  $member_name$ = value;\n"
                           "}\n");
        }
        else {
            printer->Print(variables_,
                           "- (void) Set$capitalized_name$:($storage_type$) value {\n"
                           "  has$capitalized_name$ = YES;\n"
                           "  $member_name$ = value;\n"
                           "}\n");
        }
         */
    }
    
    
    void PrimitiveFieldGenerator::GenerateBuilderMembersHeader(io::Printer* printer) const {
        //    printer->Print(variables_,
        //      "- ($classname$*) Set$capitalized_name$:($storage_type$) value;\n");
    }
    
    
    void PrimitiveFieldGenerator::GenerateMergingCodeHeader(io::Printer* printer) const {
    }
    
    
    void PrimitiveFieldGenerator::GenerateBuildingCodeHeader(io::Printer* printer) const {
    }
    
    
    void PrimitiveFieldGenerator::GenerateParsingCodeHeader(io::Printer* printer) const {
    }
    
    
    void PrimitiveFieldGenerator::GenerateSerializationCodeHeader(io::Printer* printer) const {
    }
    
    
    void PrimitiveFieldGenerator::GenerateSerializedSizeCodeHeader(io::Printer* printer) const {
    }
    
    
    void PrimitiveFieldGenerator::GenerateBuilderMembersSource(io::Printer* printer) const {
        //    printer->Print(variables_,
        //      "- (void) Set$capitalized_name$:($storage_type$) iValue {\n"
        //      "  self.has$capitalized_name$ = YES;\n"
        //      "  self.$name$ = iValue;\n"
        //      "  return self;\n"
        //      "}\n");
    }
    
    
    void PrimitiveFieldGenerator::GenerateMergingCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "if (other.has$capitalized_name$) {\n"
                       "  self.$name$ = other.$name$;\n"
                       "}\n");
    }
    
    void PrimitiveFieldGenerator::GenerateBuildingCodeSource(io::Printer* printer) const {
    }
    
    void PrimitiveFieldGenerator::GenerateParsingCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "self.$name$ = [input read$capitalized_type$];\n");
    }
    
    void PrimitiveFieldGenerator::GenerateSerializationCodeSource(io::Printer* printer) const {
        /* yanyang
        printer->Print(variables_,
                       "if (self.has$capitalized_name$) {\n"
                       "  [output write$capitalized_type$:$number$ value:self.$name$];\n"
                       "}\n");
         */
        printer->Print(variables_,
                       "if (_HAS_BIT($index$)) {\n"
                       "  [output write$capitalized_type$:$number$ value:self.$name$];\n"
                       "}\n");
    }
    
    void PrimitiveFieldGenerator::GenerateSerializedSizeCodeSource(io::Printer* printer) const {
        /* yanyang
        printer->Print(variables_,
                       "if (self.has$capitalized_name$) {\n"
                       "  iSize += compute$capitalized_type$Size($number$, self.$name$);\n"
                       "}\n");
         */
        printer->Print(variables_,
                       "if (_HAS_BIT($index$)) {\n"
                       "  iSize += compute$capitalized_type$Size($number$, self.$name$);\n"
                       "}\n");
    }
    
    void PrimitiveFieldGenerator::GenerateInitializeCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "{$number$, $field_label$, $field_type$, NO, 0, 0},\n");
    }
    
    string PrimitiveFieldGenerator::GetBoxedType() const {
        return BoxedPrimitiveTypeName(GetObjectiveCType(descriptor_));
    }
    
    
    RepeatedPrimitiveFieldGenerator::RepeatedPrimitiveFieldGenerator(const FieldDescriptor* descriptor)
    : descriptor_(descriptor) {
        SetPrimitiveVariables(descriptor, &variables_);
    }
    
    
    RepeatedPrimitiveFieldGenerator::~RepeatedPrimitiveFieldGenerator() {
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateHasFieldHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateFieldHeader(io::Printer* printer) const {
        //    printer->Print(variables_, "NSMutableArray* $mutable_list_name$;\n");
        //    if (descriptor_->options().packed()) {
        //      printer->Print(variables_,
        //        "int32_t $name$MemoizedSerializedSize;\n");
        //    }
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateHasPropertyHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GeneratePropertyHeader(io::Printer* printer) const {
        printer->Print(variables_,
                       "@property (nonatomic, strong) NSMutableArray* $name$;\n");
        
        if (HasOwnedObjectPrefixInField(variables_, "name")) {
            printer->Print(variables_,
                           "- (NSMutableArray*)$name$ PB_OBJC_METHOD_FAMILY_NONE;\n");
        }
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateExtensionSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "@property (nonatomic, strong) NSMutableArray* $mutable_list_name$;\n");
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateImplementationDeclarationSource(io::Printer* printer) const {
        //    printer->Print(variables_, "NSMutableArray* $mutable_list_name$;\n");
        if (descriptor_->options().packed()) {
            printer->Print(variables_,
                           "int32_t $name$MemoizedSerializedSize;\n");
        }
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateSynthesizeSource(io::Printer* printer) const {
        /* yanyang
         printer->Print(variables_, "@synthesize $mutable_list_name$;\n");
         printer->Print(variables_, "PB_PROPERTY_TYPE $name$;\n");
         */
        printer->Print(variables_, "PB_PROPERTY_TYPE $name$;\n");
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateDeallocSource(io::Printer* printer) const {
        printer->Print(variables_, "self.$mutable_list_name$ = nil;\n");
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateInitializationSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "self.$name$ = [[NSMutableArray alloc] init];\n");
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateMembersHeader(io::Printer* printer) const {
        printer->Print(variables_,
                       "- (NSArray*) $list_name$$list_name_method_attribute$;\n");
        
        if (HasOwnedObjectPrefixInField(variables_, "mutable_list_name")) {
            printer->Print(variables_,
                           "- (NSMutableArray*)$mutable_list_name$ PB_OBJC_METHOD_FAMILY_NONE;\n");
        }
    }
    
    void RepeatedPrimitiveFieldGenerator::GenerateBuilderMembersHeader(io::Printer* printer) const {
        //    printer->Print(variables_,
        //      "- ($classname$*) add$capitalized_name$:($storage_type$) value;\n"
        //      "- ($classname$*) add$capitalized_name$FromArray:(NSArray*) values;\n");
        
        printer->Print(variables_,
                       "- (void) add$capitalized_name$:($storage_type$) value;\n"
                       "- (void) add$capitalized_name$FromArray:(NSArray*) values;\n\n");
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateMergingCodeHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateBuildingCodeHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateParsingCodeHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateSerializationCodeHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateSerializedSizeCodeHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateMembersSource(io::Printer* printer) const {
        return;
        /* yanyang
         printer->Print(variables_,
         "- (NSArray*) $list_name$ {\n"
         "  return (nil != $mutable_list_name$) ? [NSArray arrayWithArray:$mutable_list_name$] : nil;\n"
         "}\n");
         
         printer->Print(variables_,
         "- (NSMutableArray*)$name$ {\n"
         "  if (nil == $mutable_list_name$) {\n"
         "      self.$mutable_list_name$ = [NSMutableArray array];\n"
         "  }\n"
         "  return $mutable_list_name$;\n"
         "}\n"
         "- (void)set$capitalized_name$:(NSMutableArray*) values {\n"
         "  self.$mutable_list_name$ = values;\n"
         "}\n");
         */
    }
    
    void RepeatedPrimitiveFieldGenerator::GenerateBuilderMembersSource(io::Printer* printer) const {
        printer->Print(variables_,
                       //      "- ($classname$*) add$capitalized_name$:($storage_type$) value {\n"
                       //      "  if (self.$mutable_list_name$ == nil) {\n"
                       //      "    self.$mutable_list_name$ = [NSMutableArray array];\n"
                       //      "  }\n"
                       //      "  [self.$mutable_list_name$ addObject:$boxed_value$];\n"
                       //      "  return self;\n"
                       //      "}\n"
                       //      "- ($classname$*) add$capitalized_name$FromArray:(NSArray*) values {\n"
                       //      "  if (self.$mutable_list_name$ == nil) {\n"
                       //      "    self.$mutable_list_name$ = [NSMutableArray array];\n"
                       //      "  }\n"
                       //      "  [self.$mutable_list_name$ addObjectsFromArray:values];\n"
                       //      "  return self;\n"
                       //      "}\n");
                       
                       "- (void) add$capitalized_name$:($storage_type$) value {\n"
                       "  if (self.$mutable_list_name$ == nil) {\n"
                       "    self.$mutable_list_name$ = [NSMutableArray array];\n"
                       "  }\n"
                       "  [self.$mutable_list_name$ addObject:$boxed_value$];\n"
                       "}\n"
                       "- (void) add$capitalized_name$FromArray:(NSArray*) values {\n"
                       "  if (self.$mutable_list_name$ == nil) {\n"
                       "    self.$mutable_list_name$ = [NSMutableArray array];\n"
                       "  }\n"
                       "  [self.$mutable_list_name$ addObjectsFromArray:values];\n"
                       "}\n");
    }
    
    void RepeatedPrimitiveFieldGenerator::GenerateMergingCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "if (other.$mutable_list_name$.count > 0) {\n"
                       "  if (self.$mutable_list_name$ == nil) {\n"
                       "    self.$mutable_list_name$ = [NSMutableArray array];\n"
                       "  }\n"
                       "  [self.$mutable_list_name$ addObjectsFromArray:other.$mutable_list_name$];\n"
                       "}\n");
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateBuildingCodeSource(io::Printer* printer) const {
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateParsingCodeSource(io::Printer* printer) const {
        if (descriptor_->options().packed()) {
            printer->Print(variables_,
                           "int32_t length = [input readRawVarint32];\n"
                           "int32_t limit = [input pushLimit:length];\n"
                           "while (input.bytesUntilLimit > 0) {\n"
                           //"  [self add$capitalized_name$:[input read$capitalized_type$]];\n"
                           "  $storage_type$ value = [input read$capitalized_type$];\n"
                           "  [self.$name$ addObject:$boxed_value$];\n"
                           "}\n"
                           "[input popLimit:limit];\n");
        } else {
            printer->Print(variables_,
                           //"[self add$capitalized_name$:[input read$capitalized_type$]];\n");
                           "$storage_type$ value = [input read$capitalized_type$];\n"
                           "[self.$name$ addObject:$boxed_value$];\n");
        }
    }
    
    
    void RepeatedPrimitiveFieldGenerator::GenerateSerializationCodeSource(io::Printer* printer) const {
        if (descriptor_->options().packed()) {
            printer->Print(variables_,
                           "if (self.$name$.count > 0) {\n"
                           "  [output writeRawVarint32:$tag$];\n"
                           "  [output writeRawVarint32:$name$MemoizedSerializedSize];\n"
                           "}\n");
            
            if (ReturnsPrimitiveType(descriptor_)) {
                printer->Print(variables_,
                               "for (NSNumber* value in self.$name$) {\n"
                               "  [output write$capitalized_type$NoTag:$unboxed_value$];\n"
                               "}\n");
            } else {
                printer->Print(variables_,
                               "for ($storage_type$ element in self.$name$) {\n"
                               "  [output write$capitalized_type$NoTag:element];\n"
                               "}\n");
            }
        } else {
            if (ReturnsPrimitiveType(descriptor_)) {
                printer->Print(variables_,
                               "for (NSNumber* value in self.$name$) {\n"
                               "  [output write$capitalized_type$:$number$ value:$unboxed_value$];\n"
                               "}\n");
            } else {
                printer->Print(variables_,
                               "for ($storage_type$ element in self.$name$) {\n"
                               "  [output write$capitalized_type$:$number$ value:element];\n"
                               "}\n");
            }
        }
    }
    
    void RepeatedPrimitiveFieldGenerator::GenerateSerializedSizeCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "{\n"
                       "  int32_t dataSize = 0;\n");
        printer->Indent();
        if (FixedSize(descriptor_->type()) == -1) {
            if (ReturnsPrimitiveType(descriptor_)) {
                printer->Print(variables_,
                               "for (NSNumber* value in self.$name$) {\n"
                               "  dataSize += compute$capitalized_type$SizeNoTag($unboxed_value$);\n"
                               "}\n");
            } else {
                printer->Print(variables_,
                               "for ($storage_type$ element in self.$name$) {\n"
                               "  dataSize += compute$capitalized_type$SizeNoTag(element);\n"
                               "}\n");
            }
        } else {
            printer->Print(variables_,
                           "dataSize = $fixed_size$ * self.$name$.count;\n");
        }
        
        printer->Print(
                       "iSize += dataSize;\n");
        
        if (descriptor_->options().packed()) {
            printer->Print(variables_,
                           "if (self.$name$.count > 0) {\n"
                           "  iSize += $tag_size$;\n"
                           "  iSize += computeInt32SizeNoTag(dataSize);\n"
                           "}\n"
                           "$name$MemoizedSerializedSize = dataSize;\n");
        } else {
            printer->Print(variables_,
                           "iSize += $tag_size$ * self.$name$.count;\n");
        }
        
        printer->Outdent();
        printer->Print(
                       "}\n");
    }
    
    void RepeatedPrimitiveFieldGenerator::GenerateInitializeCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "{$number$, $field_label$, $field_type$, $is_packed$, 0, 0},\n");
    }
    
    string RepeatedPrimitiveFieldGenerator::GetBoxedType() const {
        return BoxedPrimitiveTypeName(GetObjectiveCType(descriptor_));
    }
}  // namespace objectivec
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
