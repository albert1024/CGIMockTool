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

#include <map>
#include <string>

#include <google/protobuf/compiler/objectivec/objectivec_enum_field.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/compiler/objectivec/objectivec_helpers.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/wire_format.h>
#include <google/protobuf/stubs/strutil.h>

namespace google { namespace protobuf { namespace compiler { namespace objectivec {
    
    namespace {
        void SetEnumVariables(const FieldDescriptor* descriptor,
                              map<string, string>* variables) {
            const EnumValueDescriptor* default_value;
            default_value = descriptor->default_value_enum();
            
            string type = ClassName(descriptor->enum_type());
            
            const string& descriptorCamelCase = UnderscoresToCamelCase(descriptor);
            const string& descriptorCaptializedCamelCase = UnderscoresToCapitalizedCamelCase(descriptor);;
            
            (*variables)["classname"]             = ClassName(descriptor->containing_type());
            (*variables)["name"]                  = descriptorCamelCase;
            (*variables)["capitalized_name"]      = descriptorCaptializedCamelCase;
            (*variables)["list_name"]             = descriptorCamelCase + "List";
            (*variables)["mutable_list_name"] = "mutable" + descriptorCaptializedCamelCase + "List";
            (*variables)["number"] = SimpleItoa(descriptor->number());
            (*variables)["type"] = type;
            (*variables)["default"] = EnumValueName(default_value);
            (*variables)["boxed_value"] = "[NSNumber numberWithInt:value]";
            (*variables)["unboxed_value"] = "[value intValue]";
            (*variables)["tag"] = SimpleItoa(internal::WireFormat::MakeTag(descriptor));
            (*variables)["tag_size"] = SimpleItoa(
                                                  internal::WireFormat::TagSize(descriptor->number(), descriptor->type()));
            
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
    
    EnumFieldGenerator::EnumFieldGenerator(const FieldDescriptor* descriptor)
    : descriptor_(descriptor) {
        SetEnumVariables(descriptor, &variables_);
    }
    
    
    EnumFieldGenerator::~EnumFieldGenerator() {
    }
    
    
    void EnumFieldGenerator::GenerateHasFieldHeader(io::Printer* printer) const {
        // @private
        //    printer->Print(variables_, "BOOL has$capitalized_name$:1;\n");
    }
    
    
    void EnumFieldGenerator::GenerateFieldHeader(io::Printer* printer) const {
        // @private
        //    printer->Print(variables_, "$type$ $name$;\n");
    }
    
    
    void EnumFieldGenerator::GenerateHasPropertyHeader(io::Printer* printer) const {
        //    printer->Print(variables_, "- (BOOL) has$capitalized_name$;\n");
    }
    
    
    void EnumFieldGenerator::GeneratePropertyHeader(io::Printer* printer) const {
        /* yanyang
        printer->Print(variables_,
                       "@property (nonatomic, assign, readonly) BOOL has$capitalized_name$;\n"
                       "@property (nonatomic, assign, setter=Set$capitalized_name$:) $type$ $name$;\n");
         */
        printer->Print(variables_,
                       "@property (nonatomic, assign) $type$ $name$;\n");
        
        /* yanyang
        if (HasOwnedObjectPrefixInField(variables_, "name")) {
            printer->Print(variables_,
                           "- ($type$)$name$ PB_OBJC_METHOD_FAMILY_NONE;\n");
        }
         */
    }
    
    
    void EnumFieldGenerator::GenerateExtensionSource(io::Printer* printer) const {
        //    printer->Print(variables_,
        //      "@property BOOL has$capitalized_name$;\n"
        //      "@property $type$ $name$;\n");
    }
    
    
    void EnumFieldGenerator::GenerateImplementationDeclarationSource(io::Printer* printer) const {
        // yanyang
        //printer->Print(variables_, "BOOL has$capitalized_name$:1;\n");
        
        /*
        if (AreFieldsDifferent(variables_, "name", "member_name")) {
            printer->Print(variables_, "$type$ $member_name$;\n");
        }
         */
    }
    
    
    void EnumFieldGenerator::GenerateMembersHeader(io::Printer* printer) const {
    }
    
    
    void EnumFieldGenerator::GenerateMembersSource(io::Printer* printer) const {
        /* yanyang
        printer->Print(variables_,
                       "- (void) Set$capitalized_name$:($type$) value {\n"
                       "  has$capitalized_name$ = YES;\n"
                       "  $member_name$ = value;\n"
                       "}\n");
         */
    }
    
    
    void EnumFieldGenerator::GenerateSynthesizeSource(io::Printer* printer) const {
        // yanyang
        //printer->Print(variables_, "@synthesize has$capitalized_name$;\n");
        
        /*if (AreFieldsDifferent(variables_, "name", "member_name")) {
            printer->Print(variables_,
                           "@synthesize $name$ = $member_name$;\n");
        }
        else {*/
            printer->Print(variables_,
                           "PB_PROPERTY_TYPE $name$;\n");
        //}
    }
    
    
    void EnumFieldGenerator::GenerateDeallocSource(io::Printer* printer) const {
    }
    
    
    void EnumFieldGenerator::GenerateInitializationSource(io::Printer* printer) const {
        /* yanyang
        printer->Print(variables_,
                       "self.$name$ = $default$;\n"
                       "has$capitalized_name$ = NO;\n");
         */
        printer->Print(variables_,
                       "self.$name$ = $default$;\n");
    }
    
    
    void EnumFieldGenerator::GenerateBuilderMembersHeader(io::Printer* printer) const {
        //    printer->Print(variables_,
        //      "- ($classname$*) Set$capitalized_name$:($type$) iValue;\n");
    }
    
    
    void EnumFieldGenerator::GenerateBuilderMembersSource(io::Printer* printer) const {
        //    printer->Print(variables_,
        //      "- ($classname$*) Set$capitalized_name$:($type$) iValue {\n"
        //      "  self.has$capitalized_name$ = YES;\n"
        //      "  self.$name$ = iValue;\n"
        //      "  return self;\n"
        //      "}\n");
    }
    
    
    void EnumFieldGenerator::GenerateMergingCodeHeader(io::Printer* printer) const {
    }
    
    
    void EnumFieldGenerator::GenerateMergingCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "if (other.has$capitalized_name$) {\n"
                       "  self.$name$ = other.$name$;\n"
                       "}\n");
    }
    
    void EnumFieldGenerator::GenerateBuildingCodeHeader(io::Printer* printer) const {
    }
    
    
    void EnumFieldGenerator::GenerateBuildingCodeSource(io::Printer* printer) const {
    }
    
    
    void EnumFieldGenerator::GenerateParsingCodeHeader(io::Printer* printer) const {
    }
    
    
    void EnumFieldGenerator::GenerateParsingCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "int32_t iValue = [input readEnum];\n"
                       "if ($type$IsValidValue(iValue)) {\n"
                       "  self.$name$ = iValue;\n"
                       "} else {\n"
                       "  PBUnknownFieldSet_Builder* unknownFields = [PBUnknownFieldSet builder];\n"
                       "  [unknownFields mergeVarintField:$number$ value:iValue];\n"
                       "}\n");
    }
    
    
    void EnumFieldGenerator::GenerateSerializationCodeHeader(io::Printer* printer) const {
    }
    
    
    void EnumFieldGenerator::GenerateSerializationCodeSource(io::Printer* printer) const {
        /* yanyang
        printer->Print(variables_,
                       "if (self.has$capitalized_name$) {\n"
                       "  [output writeEnum:$number$ value:self.$name$];\n"
                       "}\n");
         */
        printer->Print(variables_,
                       "if (_HAS_BIT($index$)) {\n"
                       "  [output writeEnum:$number$ value:self.$name$];\n"
                       "}\n");
    }
    
    
    void EnumFieldGenerator::GenerateSerializedSizeCodeHeader(io::Printer* printer) const {
    }
    
    
    void EnumFieldGenerator::GenerateSerializedSizeCodeSource(io::Printer* printer) const {
        /* yanyang
        printer->Print(variables_,
                       "if (self.has$capitalized_name$) {\n"
                       "  iSize += computeEnumSize($number$, self.$name$);\n"
                       "}\n");
         */
        printer->Print(variables_,
                       "if (_HAS_BIT($index$)) {\n"
                       "  iSize += computeEnumSize($number$, self.$name$);\n"
                       "}\n");
    }
    
    void EnumFieldGenerator::GenerateInitializeCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "{$number$, $field_label$, $field_type$, NO, $default$, ._isEnumValidFunc = $type$IsValidValue},\n");
    }
    
    
    string EnumFieldGenerator::GetBoxedType() const {
        return ClassName(descriptor_->enum_type());
    }
    
    
    RepeatedEnumFieldGenerator::RepeatedEnumFieldGenerator(const FieldDescriptor* descriptor)
    : descriptor_(descriptor) {
        SetEnumVariables(descriptor, &variables_);
    }
    
    
    RepeatedEnumFieldGenerator::~RepeatedEnumFieldGenerator() {
    }
    
    
    void RepeatedEnumFieldGenerator::GenerateHasFieldHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedEnumFieldGenerator::GenerateFieldHeader(io::Printer* printer) const {
        //    printer->Print(variables_, "NSMutableArray* $mutable_list_name$;\n");
        //    if (descriptor_->options().packed()) {
        //      printer->Print(variables_,
        //        "int32_t $name$MemoizedSerializedSize;\n");
        //    }
    }
    
    
    void RepeatedEnumFieldGenerator::GenerateHasPropertyHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedEnumFieldGenerator::GeneratePropertyHeader(io::Printer* printer) const {
        printer->Print(variables_,
                       "@property (nonatomic, strong) NSMutableArray* $name$;\n");
        
        if (HasOwnedObjectPrefixInField(variables_, "name")) {
            printer->Print(variables_,
                           "- (NSMutableArray*)$name$ PB_OBJC_METHOD_FAMILY_NONE;\n");
        }
    }
    
    
    void RepeatedEnumFieldGenerator::GenerateExtensionSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "@property (nonatomic, strong) NSMutableArray* $mutable_list_name$;\n");
    }
    
    
    void RepeatedEnumFieldGenerator::GenerateImplementationDeclarationSource(io::Printer* printer) const {
        //    printer->Print(variables_, "NSMutableArray* $mutable_list_name$;\n");
        if (descriptor_->options().packed()) {
            printer->Print(variables_,
                           "int32_t $name$MemoizedSerializedSize;\n");
        }
    }
    
    
    void RepeatedEnumFieldGenerator::GenerateSynthesizeSource(io::Printer* printer) const {
        /* yanyang
        printer->Print(variables_,
                       "@synthesize $mutable_list_name$;\n");
        
        printer->Print(variables_,
                       "PB_PROPERTY_TYPE $name$;\n");
         */
        printer->Print(variables_,
                       "PB_PROPERTY_TYPE $name$;\n");
    }
    
    void RepeatedEnumFieldGenerator::GenerateDeallocSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "self.$mutable_list_name$ = nil;\n");
    }
    
    
    void RepeatedEnumFieldGenerator::GenerateInitializationSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "self.$name$ = [[NSMutableArray alloc] init];\n");
    }
    
    
    void RepeatedEnumFieldGenerator::GenerateMembersHeader(io::Printer* printer) const {
        printer->Print(variables_,
                       "- (NSArray*) $list_name$$list_name_method_attribute$;\n");
        
        if (HasOwnedObjectPrefixInField(variables_, "mutable_list_name")) {
            printer->Print(variables_,
                           "- (NSMutableArray*)$mutable_list_name$ PB_OBJC_METHOD_FAMILY_NONE;\n");
        }
    }
    
    void RepeatedEnumFieldGenerator::GenerateBuilderMembersHeader(io::Printer* printer) const {
        printer->Print(variables_,
                       "- (void) add$capitalized_name$:($type$) value;\n"
                       "- (void) add$capitalized_name$FromArray:(NSArray*) values;\n\n");
    }
    
    
    void RepeatedEnumFieldGenerator::GenerateMergingCodeHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedEnumFieldGenerator::GenerateBuildingCodeHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedEnumFieldGenerator::GenerateParsingCodeHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedEnumFieldGenerator::GenerateSerializationCodeHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedEnumFieldGenerator::GenerateSerializedSizeCodeHeader(io::Printer* printer) const {
    }
    
    
    void RepeatedEnumFieldGenerator::GenerateMembersSource(io::Printer* printer) const {
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
    
    void RepeatedEnumFieldGenerator::GenerateBuilderMembersSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "- (void) add$capitalized_name$:($type$) iValue {\n"
                       "  if (self.$mutable_list_name$ == nil) {\n"
                       "    self.$mutable_list_name$ = [NSMutableArray array];\n"
                       "  }\n"
                       "  [self.$mutable_list_name$ addObject:[NSNumber numberWithInt:iValue]];\n"
                       "}\n"
                       "- (void) add$capitalized_name$FromArray:(NSArray*) values {\n"
                       "  if (self.$mutable_list_name$ == nil) {\n"
                       "    self.$mutable_list_name$ = [NSMutableArray array];\n"
                       "  }\n"
                       "  [self.$mutable_list_name$ addObjectsFromArray:values];\n"
                       "}\n");
    }
    
    void RepeatedEnumFieldGenerator::GenerateMergingCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "if (other.$mutable_list_name$.count > 0) {\n"
                       "  if (self.$mutable_list_name$ == nil) {\n"
                       "    self.$mutable_list_name$ = [NSMutableArray array];\n"
                       "  }\n"
                       "  [self.$mutable_list_name$ addObjectsFromArray:other.$mutable_list_name$];\n"
                       "}\n");
    }
    
    void RepeatedEnumFieldGenerator::GenerateBuildingCodeSource(io::Printer* printer) const {
    }
    
    void RepeatedEnumFieldGenerator::GenerateParsingCodeSource(io::Printer* printer) const {
        // If packed, set up the while loop
        if (descriptor_->options().packed()) {
            printer->Print(variables_,
                           "int32_t length = [input readRawVarint32];\n"
                           "int32_t oldLimit = [input pushLimit:length];\n"
                           "while (input.bytesUntilLimit > 0) {\n");
            printer->Indent();
        }
        
        printer->Print(variables_,
                       "int32_t iValue = [input readEnum];\n"
                       "if ($type$IsValidValue(iValue)) {\n"
                       //"  [self add$capitalized_name$:iValue];\n"
                       "  [self.$name$ addObject:[NSNumber numberWithInt:iValue]];\n"
                       "} else {\n"
                       "  PBUnknownFieldSet_Builder* unknownFields = [PBUnknownFieldSet builder];\n"
                       "  [unknownFields mergeVarintField:$number$ value:iValue];\n"
                       "}\n");
        
        if (descriptor_->options().packed()) {
            printer->Outdent();
            printer->Print(variables_,
                           "}\n"
                           "[input popLimit:oldLimit];\n");
        }
    }
    
    void RepeatedEnumFieldGenerator::GenerateSerializationCodeSource(io::Printer* printer) const {
        /* yanyang
        if (descriptor_->options().packed()) {
            printer->Print(variables_,
                           "if (self.$mutable_list_name$.count > 0) {\n"
                           "  [output writeRawVarint32:$tag$];\n"
                           "  [output writeRawVarint32:$name$MemoizedSerializedSize];\n"
                           "}\n"
                           "for (NSNumber* element in self.$mutable_list_name$) {\n"
                           "  [output writeEnumNoTag:element.intValue];\n"
                           "}\n");
        } else {
            printer->Print(variables_,
                           "for (NSNumber* element in self.$mutable_list_name$) {\n"
                           "  [output writeEnum:$number$ value:element.intValue];\n"
                           "}\n");
        } 
         */
        if (descriptor_->options().packed()) {
            printer->Print(variables_,
                           "if (self.$name$.count > 0) {\n"
                           "  [output writeRawVarint32:$tag$];\n"
                           "  [output writeRawVarint32:$name$MemoizedSerializedSize];\n"
                           "}\n"
                           "for (NSNumber* element in self.$name$) {\n"
                           "  [output writeEnumNoTag:element.intValue];\n"
                           "}\n");
        } else {
            printer->Print(variables_,
                           "for (NSNumber* element in self.$name$) {\n"
                           "  [output writeEnum:$number$ value:element.intValue];\n"
                           "}\n");
        }
    }
    
    
    void RepeatedEnumFieldGenerator::GenerateSerializedSizeCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "{\n"
                       "  int32_t dataSize = 0;\n");
        printer->Indent();
        
        /* yanyang
        printer->Print(variables_,
                       "for (NSNumber* element in self.$mutable_list_name$) {\n"
                       "  dataSize += computeEnumSizeNoTag(element.intValue);\n"
                       "}\n");
         */
        printer->Print(variables_,
                       "for (NSNumber* element in self.$name$) {\n"
                       "  dataSize += computeEnumSizeNoTag(element.intValue);\n"
                       "}\n");
        
        printer->Print(
                       "iSize += dataSize;\n");
        
        if (descriptor_->options().packed()) {
            printer->Print(variables_,
                           "if (self.$name$.count > 0) {\n"
                           "  iSize += $tag_size$;\n"
                           "  iSize += computeRawVarint32Size(dataSize);\n"
                           "}\n");
        } else {
            printer->Print(variables_,
                           "iSize += $tag_size$ * self.$name$.count;\n");
        }
        
        if (descriptor_->options().packed()) {
            printer->Print(variables_,
                           "$name$MemoizedSerializedSize = dataSize;\n");
        }
        
        printer->Outdent();
        printer->Print("}\n");
    }
    
    
    void RepeatedEnumFieldGenerator::GenerateInitializeCodeSource(io::Printer* printer) const {
        printer->Print(variables_,
                       "{$number$, $field_label$, $field_type$, $is_packed$, 0, ._isEnumValidFunc = $type$IsValidValue},\n");
    }
    
    string RepeatedEnumFieldGenerator::GetBoxedType() const {
        return ClassName(descriptor_->enum_type());
    }
}  // namespace objectivec
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
