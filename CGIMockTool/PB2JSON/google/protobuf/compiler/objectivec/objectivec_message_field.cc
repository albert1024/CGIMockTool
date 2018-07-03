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

#include <google/protobuf/compiler/objectivec/objectivec_message_field.h>
#include <google/protobuf/compiler/objectivec/objectivec_helpers.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/wire_format.h>
#include <google/protobuf/stubs/strutil.h>

namespace google { namespace protobuf { namespace compiler { namespace objectivec {
    
    namespace {
        void SetMessageVariables(const FieldDescriptor* descriptor,
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
            (*variables)["type"] = ClassName(descriptor->message_type());
            if (IsPrimitiveType(objc_type)) {
                (*variables)["storage_type"] = ClassName(descriptor->message_type());
            } else {
                (*variables)["storage_type"] = string(ClassName(descriptor->message_type())) + "*";
            }
            (*variables)["group_or_message"] =
            (descriptor->type() == FieldDescriptor::TYPE_GROUP) ?
            "Group" : "Message";
            
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
            
            (*variables)["member_name"] = descriptorCamelCase;
            if (IsObjectiveCKeyWord(descriptorCamelCase)
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
    
    
    MessageFieldGenerator::MessageFieldGenerator(const FieldDescriptor* descriptor)
    : descriptor_(descriptor) {
        SetMessageVariables(descriptor, &variables_);
    }
    
    
    MessageFieldGenerator::~MessageFieldGenerator() {
    }
    
    
    void MessageFieldGenerator::GenerateHasFieldHeader(io::Printer* printer) const {
        //    printer->Print(variables_, "BOOL has$capitalized_name$:1;\n");
    }
    
    
    void MessageFieldGenerator::GenerateFieldHeader(io::Printer* printer) const {
        //    printer->Print(variables_, "$storage_type$ $name$;\n");
    }
    
    
    void MessageFieldGenerator::GenerateHasPropertyHeader(io::Printer* printer) const {
        //   printer->Print(variables_, "- (BOOL) has$capitalized_name$;\n");
    }
    
    
    void MessageFieldGenerator::GeneratePropertyHeader(io::Printer* printer) const {
        printer->Print(variables_,
                       "@property (nonatomic, strong) $storage_type$ $name$;\n");
        
        if (HasOwnedObjectPrefixInField(variables_, "name")) {
            printer->Print(variables_,
                           "- ($storage_type$)$name$ PB_OBJC_METHOD_FAMILY_NONE;\n");
        }
    }
    
    
    void MessageFieldGenerator::GenerateExtensionSource(io::Printer* printer) const {
        //    printer->Print(variables_,
        //      "@property BOOL has$capitalized_name$;\n"
        //      "@property (strong) $storage_type$ $name$;\n");
    }
    
    
    void MessageFieldGenerator::GenerateImplementationDeclarationSource(io::Printer* printer) const {
        // yanyang
        //printer->Print(variables_, "BOOL has$capitalized_name$:1;\n");
        
        /*
        if (AreFieldsDifferent(variables_, "name", "member_name")) {
            printer->Print(variables_, "$storage_type$ $member_name$;\n");
        }
         */
    }
    
    
    void MessageFieldGenerator::GenerateMembersHeader(io::Printer* printer) const {
    }
    
    
    void MessageFieldGenerator::GenerateSynthesizeSource(io::Printer* printer) const {
        /*if (AreFieldsDifferent(variables_, "name", "member_name")) {
            printer->Print(variables_,
                           //"@synthesize has$capitalized_name$;\n"
                           "@synthesize $name$ = $member_name$;\n");
        }
        else {*/
            printer->Print(variables_,
                           "PB_PROPERTY_TYPE $name$;\n");
    }
    
    
    void MessageFieldGenerator::GenerateDeallocSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "self.$name$ = nil;\n");
    }
    
    
    void MessageFieldGenerator::GenerateInitializationSource(io::Printer* printer) const {
        //printer->Print(variables_, "self.$name$ = [$type$ defaultInstance];\n");
        
        /* yanyang
        printer->Print(variables_,
                       "self.$name$ = nil;\n"
                       "has$capitalized_name$ = NO;\n");
         */
    }
    
    
    void MessageFieldGenerator::GenerateBuilderMembersHeader(io::Printer* printer) const {
        //    printer->Print(variables_,
        //      "- ($classname$*) Set$capitalized_name$:($storage_type$) value;\n");
    }
    
    void MessageFieldGenerator::GenerateBuilderMembersSource(io::Printer* printer) const {
        //    printer->Print(variables_,
        //      "- ($classname$*) Set$capitalized_name$:($storage_type$) oValue {\n"
        //	  "  self.has$capitalized_name$ = YES;\n"
        //      "  self.$name$ = oValue;\n"
        //      "  return self;\n"
        //      "}\n");
    }
    
    
    void MessageFieldGenerator::GenerateMergingCodeHeader(io::Printer* printer) const {
    }
    
    
    void MessageFieldGenerator::GenerateMergingCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "if (other.has$capitalized_name$) {\n"
                       "  [self merge$capitalized_name$:other.$name$];\n"
                       "}\n");
    }
    
    
    void MessageFieldGenerator::GenerateBuildingCodeHeader(io::Printer* printer) const {
    }
    
    
    void MessageFieldGenerator::GenerateBuildingCodeSource(io::Printer* printer) const {
    }
    
    
    void MessageFieldGenerator::GenerateParsingCodeHeader(io::Printer* printer) const {
    }
    
    
    void MessageFieldGenerator::GenerateParsingCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "$type$* subBuilder = [[$type$ alloc] init];\n");
        
        if (descriptor_->type() == FieldDescriptor::TYPE_GROUP) {
            printer->Print(variables_,
                           "[input readGroup:$number$ builder:subBuilder];\n");
        } else {
            printer->Print(variables_,
                           "[input readMessage:subBuilder];\n");
        }
        
        printer->Print(variables_,
                       "self.$name$ = subBuilder;\n");
    }
    
    
    void MessageFieldGenerator::GenerateSerializationCodeHeader(io::Printer* printer) const {
    }
    
    
    void MessageFieldGenerator::GenerateSerializationCodeSource(io::Printer* printer) const {
        /* yanyang
        printer->Print(variables_,
                       "if (self.has$capitalized_name$) {\n"
                       "  [output write$group_or_message$:$number$ value:self.$name$];\n"
                       "}\n");
         */
        printer->Print(variables_,
                       "if (_HAS_BIT($index$)) {\n"
                       "  [output write$group_or_message$:$number$ value:self.$name$];\n"
                       "}\n");
    }
    
    
    void MessageFieldGenerator::GenerateSerializedSizeCodeHeader(io::Printer* printer) const {
    }
    
    
    void MessageFieldGenerator::GenerateSerializedSizeCodeSource(io::Printer* printer) const {
        /* yanyang
        printer->Print(variables_,
                       "if (self.has$capitalized_name$) {\n"
                       "  iSize += compute$group_or_message$Size($number$, self.$name$);\n"
                       "}\n");
         */
        printer->Print(variables_,
                       "if (_HAS_BIT($index$)) {\n"
                       "  iSize += compute$group_or_message$Size($number$, self.$name$);\n"
                       "}\n");
    }
    
    void MessageFieldGenerator::GenerateMembersSource(io::Printer* printer) const {
        /* yanyang
        printer->Print(variables_,
                       "- (void) Set$capitalized_name$:($storage_type$) value {\n"
                       "  has$capitalized_name$ = YES;\n"
                       "  $member_name$ = value;\n"
                       "}\n");
         */
    }
    
    void MessageFieldGenerator::GenerateInitializeCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "{$number$, $field_label$, $field_type$, NO, 0, ._messageClassName = STRING_FROM($type$)},\n");
    }
    
    
    string MessageFieldGenerator::GetBoxedType() const {
        return ClassName(descriptor_->message_type());
    }
    
    
    RepeatedMessageFieldGenerator::RepeatedMessageFieldGenerator(const FieldDescriptor* descriptor)
    : descriptor_(descriptor) {
        SetMessageVariables(descriptor, &variables_);
    }
    
    
    RepeatedMessageFieldGenerator::~RepeatedMessageFieldGenerator() {
    }
    
    
    void RepeatedMessageFieldGenerator::GenerateHasFieldHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedMessageFieldGenerator::GenerateFieldHeader(io::Printer* printer) const {
        //    printer->Print(variables_, "NSMutableArray* $mutable_list_name$;\n");
    }
    
    
    void RepeatedMessageFieldGenerator::GenerateHasPropertyHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedMessageFieldGenerator::GeneratePropertyHeader(io::Printer* printer) const {
        printer->Print(variables_,
                       "@property (nonatomic, strong) NSMutableArray* $name$;\n");
        
        if (HasOwnedObjectPrefixInField(variables_, "name")) {
            printer->Print(variables_,
                           "- (NSMutableArray*)$name$ PB_OBJC_METHOD_FAMILY_NONE;\n");
        }
    }
    
    
    void RepeatedMessageFieldGenerator::GenerateExtensionSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "@property (nonatomic, strong) NSMutableArray* $mutable_list_name$;\n");
    }
    
    
    void RepeatedMessageFieldGenerator::GenerateImplementationDeclarationSource(io::Printer* printer) const {
        //    printer->Print(variables_, "NSMutableArray* $mutable_list_name$;\n");
    }
    
    
    void RepeatedMessageFieldGenerator::GenerateSynthesizeSource(io::Printer* printer) const {
        /* yanyang
        printer->Print(variables_,
                       "@synthesize $mutable_list_name$;\n"
                       "PB_PROPERTY_TYPE $name$;\n");
         */
        printer->Print(variables_,
                       "PB_PROPERTY_TYPE $name$;\n");
    }
    
    
    void RepeatedMessageFieldGenerator::GenerateDeallocSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "self.$mutable_list_name$ = nil;\n");
    }
    
    
    void RepeatedMessageFieldGenerator::GenerateInitializationSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "self.$name$ = [[NSMutableArray alloc] init];\n");
    }
    
    
    void RepeatedMessageFieldGenerator::GenerateMembersHeader(io::Printer* printer) const {
        printer->Print(variables_,
                       "- (NSArray*) $list_name$$list_name_method_attribute$;\n");
        
        if (HasOwnedObjectPrefixInField(variables_, "mutable_list_name")) {
            printer->Print(variables_,
                           "- (NSMutableArray*)$mutable_list_name$ PB_OBJC_METHOD_FAMILY_NONE;\n");
        }
    }
    
    void RepeatedMessageFieldGenerator::GenerateMembersSource(io::Printer* printer) const {
        return;
        /* yanyang
        printer->Print(variables_,
                       "- (NSArray*) $list_name$ {\n"
                       "  return (nil != $mutable_list_name$) ? [NSArray arrayWithArray:$mutable_list_name$] : nil;\n"
                       "}\n"
                       "-(NSMutableArray*)$name$ {\n"
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
    
    
    void RepeatedMessageFieldGenerator::GenerateBuilderMembersHeader(io::Printer* printer) const {
        printer->Print(variables_,
                       "- (void) add$capitalized_name$:($storage_type$) value;\n"
                       "- (void) add$capitalized_name$FromArray:(NSArray*) values;\n\n");
    }
    
    void RepeatedMessageFieldGenerator::GenerateBuilderMembersSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "- (void) add$capitalized_name$FromArray:(NSArray*) values {\n"
                       "  if (self.$mutable_list_name$ == nil) {\n"
                       "    self.$mutable_list_name$ = [NSMutableArray array];\n"
                       "  }\n"
                       "  [self.$mutable_list_name$ addObjectsFromArray:values];\n"
                       "}\n");
        
        printer->Print(
                       variables_,
                       "- (void) add$capitalized_name$:($storage_type$) value {\n"
                       "  if (self.$mutable_list_name$ == nil) {\n"
                       "    self.$mutable_list_name$ = [NSMutableArray array];\n"
                       "  }\n"
                       "  [self.$mutable_list_name$ addObject:$boxed_value$];\n"
                       "}\n");
    }
    
    
    void RepeatedMessageFieldGenerator::GenerateMergingCodeHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedMessageFieldGenerator::GenerateBuildingCodeHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedMessageFieldGenerator::GenerateParsingCodeHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedMessageFieldGenerator::GenerateSerializationCodeHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedMessageFieldGenerator::GenerateSerializedSizeCodeHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedMessageFieldGenerator::GenerateMergingCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "if (other.$mutable_list_name$.count > 0) {\n"
                       "  if (self.$mutable_list_name$ == nil) {\n"
                       "    self.$mutable_list_name$ = [NSMutableArray array];\n"
                       "  }\n"
                       "  [self.$mutable_list_name$ addObjectsFromArray:other.$mutable_list_name$];\n"
                       "}\n");
    }
    
    
    void RepeatedMessageFieldGenerator::GenerateBuildingCodeSource(io::Printer* printer) const {
    }
    
    void RepeatedMessageFieldGenerator::GenerateParsingCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "$type$* subBuilder = [[$type$ alloc] init];\n");
        
        if (descriptor_->type() == FieldDescriptor::TYPE_GROUP) {
            printer->Print(variables_,
                           "[input readGroup:$number$ builder:subBuilder];\n");
        } else {
            printer->Print(variables_,
                           "[input readMessage:subBuilder];\n");
        }
        
        printer->Print(variables_,
                       "[self.$name$ addObject:subBuilder];\n");
                       //"[self add$capitalized_name$:subBuilder];\n");
    }
    
    void RepeatedMessageFieldGenerator::GenerateSerializationCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "for ($type$* element in self.$name$) {\n"
                       "  [output write$group_or_message$:$number$ value:element];\n"
                       "}\n");
    }
    
    void RepeatedMessageFieldGenerator::GenerateSerializedSizeCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "for ($type$* element in self.$name$) {\n"
                       "  iSize += compute$group_or_message$Size($number$, element);\n"
                       "}\n");
    }
    
    void RepeatedMessageFieldGenerator::GenerateInitializeCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "{$number$, $field_label$, $field_type$, $is_packed$, ._messageClassName = STRING_FROM($type$)},\n");
    }
    
    string RepeatedMessageFieldGenerator::GetBoxedType() const {
        return ClassName(descriptor_->message_type());
    }
}  // namespace objectivec
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
