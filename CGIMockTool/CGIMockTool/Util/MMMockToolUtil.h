//
//  MMMockToolUtil.h
//  CGIMockTool
//
//  Created by littleliang on 2018/6/9.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>


@class MMCgiModel;
@class MMCgiMockScript;
@class MMCgiMockCase;
static NSString * const kNotificationSaveRootDir = @"kNotificationSaveRootDir";

@interface MMMockToolUtil : NSObject

+ (NSString *)projectRootDir;

+ (NSString *)workSpaceDir;

+ (NSString *)mockcaseRootDir;

+ (void)saveProjectRootDir:(NSString *)projectDir;

+ (NSArray *)protoFiles;

+ (NSArray *)parseProtoFilesToCgiModel:(NSArray *)protoFiles;

+ (NSArray *)parseCrossplatformProtoFilesToCgiModel;

+ (NSString *)generateMockCaseTemplateWithCgiModel:(MMCgiModel *)cgiModel;

+ (BOOL)saveMockScript:(MMCgiMockScript *)mockScript;

+ (NSMutableArray *)getMockScripts;

+ (BOOL)saveMockCase:(MMCgiMockCase *)mockCase;

+ (NSMutableArray *)getMockCases;

+ (BOOL)removeMockCase:(MMCgiMockCase *)mockCase;

+ (BOOL)createMockCase:(NSString *)mockCaseName;

+ (void)showAlert:(NSWindow *)window title:(NSString *)title message:(NSString *)message;

@end
