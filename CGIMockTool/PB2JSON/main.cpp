//
//  main.cpp
//  PB2JSON
//
//  Created by littleliang on 2018/4/28.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#include <stdio.h>
#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/compiler/objectivec/objectivec_generator.h>


int main(int argc, char *argv[]) {
//    ProtobufMsg message;
    
//    argc = 6;
//
//    char **argv1 = new char *[3];
//    argv1[0] = new char[200];
//    strcpy(argv1[0], argv[0]);
//
////    /Users/lianglitu/Developer/Wechat_Git/wechat_trunk/wechat/3rd/protobuf/mmbuiltintype.proto /Users/lianglitu/Developer/Wechat_Git/wechat_trunk/wechat/3rd/protobuf/mmpayappsvr.proto /Users/lianglitu/Developer/Wechat_Git/wechat_trunk/wechat/3rd/protobuf/mmpayappsvr.proto --target=GetEUInfoResp
//    argv1[1] = new char[200];
//    strcpy(argv1[1], "/Users/lianglitu/Developer/Wechat_Git/wechat_trunk/wechat/MMFoundation/MMSvrKit/CommNew/skproto/mmbuiltintype.proto");
////
//    argv1[2] = new char[200];
//    strcpy(argv1[2], "/Users/lianglitu/Developer/Wechat_Git/wechat_trunk/wechat/MMFoundation/MMSvrKit/CommNew/mmpay/mmpayappsvr.proto");
////
//
//    argv1[3] = new char[200];
//    strcpy(argv1[3], "/Users/lianglitu/Developer/Wechat_Git/wechat_trunk/wechat/MMFoundation/MMSvrKit/CommNew/mmpay/mmpayappsvr.proto");
//
//
//    argv1[4] = new char[200];
//    strcpy(argv1[4], "--target=GetEUInfoResp");
//
//    argv1[5] = new char[200];
//    strcpy(argv1[5], "--mockcase_out=./");
//
    google::protobuf::compiler::CommandLineInterface cli;
    
    google::protobuf::compiler::objectivec::ObjectiveCGenerator objc_generator;
    cli.RegisterGenerator("--mockcase_out", &objc_generator, "Generate Mock Case Script File");
    
    return cli.Run(argc, argv);
}
