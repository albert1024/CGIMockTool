 //
//  MMMockToolUtil.m
//  CGIMockTool
//
//  Created by littleliang on 2018/6/9.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import "MMMockToolUtil.h"
#import "MMCgiModel.h"
#import "MMCgiMockCase.h"
#include <stdio.h>
#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/compiler/objectivec/objectivec_generator.h>


#define MMBIZPROTODEF @"mmbizprotodef.h"
#define MMOCHWAPPDEF @"mmochwappdef.h"
#define MMUXPROTODEF @"mmuxprotodef.h"
#define MMPAYPROTODEF @"mmpayprotodef.h"
#define MMOCGAMESPROTODEF @"mmocgameappdef.h"
#define MMOCADPROTODEF @"mmocadappdef.h"
#define MMROTODEF @"mmprotodef.h"

#define MOCKSCRIPTSDATA @"mockscripts.dat"
#define MOCKCASEPLIST @"MockCase.plist"

#define MOCKCASEARRAYKEY @"MockCases"

static NSString * const kRootDirKey = @"rootdirkey1";
static NSUserDefaults *userDefaults = nil;
static dispatch_once_t onceToken;
static NSMutableArray *kProtofilePathsArray = nil;
static NSMutableDictionary * kCgiFuncNameMessageNameMap = nil;
static NSMutableDictionary * kMessageNameProtoFileMap = nil;


@implementation MMMockToolUtil

+ (NSString *)projectRootDir {
    //NSUserDefaults *
    NSString *bundlePath = [[NSBundle mainBundle] bundlePath];
    NSString *defaultRootPath = [[[bundlePath stringByDeletingLastPathComponent] stringByDeletingLastPathComponent] stringByDeletingLastPathComponent];
    return defaultRootPath;
}

+ (void)saveProjectRootDir:(NSString *)projectDir {
    dispatch_once(&onceToken, ^{
        userDefaults = [NSUserDefaults standardUserDefaults];
    });
    [userDefaults setObject:projectDir forKey:kRootDirKey];
}

+ (NSString *)workSpaceDir {
    return [[self projectRootDir] stringByAppendingPathComponent:@"/tools/CgiMockTool"];
}

+ (NSString *)mockcaseRootDir {
    return [[self projectRootDir] stringByAppendingPathComponent:@"/MMTest/CgiMock/CgiMockTestCase"];
}

+ (NSArray *)protoFiles {
    if ([self projectRootDir].length == 0) {
        return nil;
    }
    NSString *prefixDir = [[self projectRootDir] stringByAppendingString:@"/MMFoundation/MMSvrkit/CommNew/"];
    
    NSString *mmbizproto = [prefixDir stringByAppendingString:MMBIZPROTODEF];
    NSString *mmochwappdef = [prefixDir stringByAppendingString:MMOCHWAPPDEF];
    NSString *mmuxproto = [prefixDir stringByAppendingString:MMUXPROTODEF];
    NSString *mmpayproto = [prefixDir stringByAppendingString:MMPAYPROTODEF];
    NSString *mmgameproto = [prefixDir stringByAppendingString:MMOCGAMESPROTODEF];
    NSString *mmadproto = [prefixDir stringByAppendingString:MMOCADPROTODEF];
    NSString *mmproto = [prefixDir stringByAppendingString:MMROTODEF];
    
    return @[mmbizproto, mmochwappdef, mmuxproto, mmgameproto, mmadproto, mmproto, mmpayproto];
}

+ (NSArray *)parseProtoFilesToCgiModel:(NSArray *)protoFiles {
    [self loadProtofileArrays];
    [self loadCgiFuncNameMessageNameMap];
    [self loadMessageNameProtoFileMap];
    NSMutableArray *cgiModelArray = [[NSMutableArray alloc] init];
    for (NSString *protoFileName in protoFiles) {
        NSError *error = nil;
        NSString *fileContent = [NSString stringWithContentsOfFile:protoFileName encoding:NSUTF8StringEncoding error:&error];
        NSArray *lines = [fileContent componentsSeparatedByString:@"\n"];
        NSEnumerator *enumator = [lines objectEnumerator];
        NSString *tmp = nil;
        while (tmp = [enumator nextObject]) {
            if ([tmp containsString:@"MMFunc_"]) {
                NSRange range = [tmp rangeOfString:@"MMFunc_"];
                NSUInteger startLocation = range.location;
                range = [tmp rangeOfString:@","];
                NSUInteger endLocation = range.location;
                if (endLocation == NSNotFound) {
                    continue;
                }
                NSString *subStr = [tmp substringWithRange:NSMakeRange(startLocation, (endLocation - startLocation))];
                NSArray *componets = [subStr componentsSeparatedByString:@"="];
                if (componets.count != 2) {
                    continue;
                }
                NSString *cgiFuncName = [(NSString *)[componets objectAtIndex:0] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
                NSString *cgiFuncNumber = [(NSString *)[componets objectAtIndex:1] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
                MMCgiModel *cgiModel = [[MMCgiModel alloc] init];
                cgiModel.cgiNumber = (UInt32)[cgiFuncNumber integerValue];
                cgiModel.cgiFuncName = cgiFuncName;
                cgiModel.messageName = [kCgiFuncNameMessageNameMap objectForKey:cgiFuncName];
                cgiModel.protoPath = [kMessageNameProtoFileMap objectForKey:cgiModel.messageName];
                if ([self isModelValid:cgiModel]) {
                    [cgiModelArray addObject:cgiModel];
                }
            }
        }
    }
    [cgiModelArray sortUsingComparator:^NSComparisonResult(id  _Nonnull obj1, id  _Nonnull obj2) {
        MMCgiModel *model1 = (MMCgiModel *)obj1;
        MMCgiModel *model2 = (MMCgiModel *)obj2;
        if (model1.cgiNumber > model2.cgiNumber) {
            return NSOrderedDescending;
        } else if (model1.cgiNumber < model2.cgiNumber) {
            return NSOrderedAscending;
        } else {
            return NSOrderedSame;
        }
    }];
    return cgiModelArray;
}

+ (BOOL)isModelValid:(MMCgiModel *)model {
    if (model.messageName.length > 0 && model.cgiNumber > 0 && model.cgiFuncName.length > 0 && model.protoPath.length > 0) {
        return YES;
    }
    return NO;
}

+ (void)loadMessageNameProtoFileMap {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        kMessageNameProtoFileMap = [[NSMutableDictionary alloc] init];
    });
    [kMessageNameProtoFileMap removeAllObjects];
    
    if (kProtofilePathsArray.count == 0) {
        [self loadProtofileArrays];
    }
    for (NSString *nsProtofileName in kProtofilePathsArray) {
        NSError *error = nil;
        NSString *fileContent = [NSString stringWithContentsOfFile:nsProtofileName encoding:NSUTF8StringEncoding error:&error];
        NSArray *lines = [fileContent componentsSeparatedByString:@"\n"];
        NSEnumerator *enumator = [lines objectEnumerator];
        NSString *tmp = nil;
        while (tmp = [enumator nextObject]) {
            if ([tmp containsString:@"message"]) {
                NSString *trimmingString = [tmp stringByTrimmingCharactersInSet:
                                            [NSCharacterSet whitespaceAndNewlineCharacterSet]];
                if ([[trimmingString substringWithRange:NSMakeRange(0, 7)] isEqualToString:@"message"]) {
                    NSArray *messageComponents = [trimmingString componentsSeparatedByString:@" "];
                    NSMutableArray *messageComponentsMutableArray = [messageComponents mutableCopy];
                    [messageComponentsMutableArray enumerateObjectsUsingBlock:^(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
                        if ([(NSString *)obj length] == 0) {
                            [messageComponentsMutableArray removeObject:obj];
                        }
                    }];
                    NSString *messageName = [messageComponentsMutableArray objectAtIndex:1];
                    if ([messageName containsString:@"{"]) {
                        NSRange range = [messageName rangeOfString:@"{"];
                        messageName = [messageName substringToIndex:range.location];
                    } else if ([messageName containsString:@"/"]) {
                        NSRange range = [messageName rangeOfString:@"/"];
                        messageName = [messageName substringToIndex:range.location];
                    }
                    messageName = [messageName stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
                    [kMessageNameProtoFileMap setObject:nsProtofileName forKey:messageName];
                }
            }
        }
    }
}



+ (void)loadCgiFuncNameMessageNameMap {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        kCgiFuncNameMessageNameMap = [[NSMutableDictionary alloc] init];
    });
    [kCgiFuncNameMessageNameMap removeAllObjects];
    NSString *nsCgiEventConfigPath = [self cgiEventConfigPath];
    NSError *error = nil;
    NSString *fileContent = [NSString stringWithContentsOfFile:nsCgiEventConfigPath encoding:NSUTF8StringEncoding error:&error];
    NSArray *lines = [fileContent componentsSeparatedByString:@"\n"];
    NSEnumerator *enumator = [lines objectEnumerator];
    NSString *tmp = nil;
    while (tmp = [enumator nextObject]) {
        if ([tmp containsString:@"PBW("]) {
            NSString *tstString = [tmp stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
            if ([tstString hasPrefix:@"//"]) {
                continue;
            }
            NSRange range = [tmp rangeOfString:@"PBW("];
            NSString *tmpSubstring = [tmp substringFromIndex:range.location + range.length];
            NSArray *components = [tmpSubstring componentsSeparatedByString:@","];
            NSString *funcName = nil;
            NSString *messageName = nil;
            for (NSString *component in components) {
                if ([component containsString:@"MMFunc"]) {
                    funcName = [component stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
                } else if ([component containsString:@" class"]) {
                    NSString *targetMessageName = nil;
                    NSArray *messageNameComponents = [component componentsSeparatedByString:@" "];
                    for (NSString *messageNameComponent in messageNameComponents) {
                        if ([messageNameComponent containsString:@"["]) {
                            targetMessageName = messageNameComponent;
                            break;
                        }
                    }
                    messageName = [[targetMessageName stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]] substringFromIndex:1];;
                    break;
                }
            }
            if (funcName.length > 0 && messageName.length > 0) {
             [kCgiFuncNameMessageNameMap setObject:messageName forKey:funcName];
            }
        }
    }
}

+ (void)loadProtofileArrays {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        kProtofilePathsArray = [[NSMutableArray alloc] init];
    });
    [kProtofilePathsArray removeAllObjects];
    NSString *protoDir = [[self projectRootDir] stringByAppendingPathComponent:@"/MMFoundation/MMSvrKit/CommNew"];
    NSMutableArray *filesArray = [self getAllFiles:protoDir];
    for (NSString *nsFilePath in filesArray) {
        if ([[nsFilePath pathExtension] isEqualToString:@"proto"]) {
            [kProtofilePathsArray addObject:nsFilePath];
        }
    }
    return;
}

+ (NSArray *)getCurrentFiles:(NSString *)root {
    NSFileManager* fm;
    fm = [NSFileManager defaultManager];
    
    return [fm contentsOfDirectoryAtPath:root error:nil];
}

+ (NSMutableArray*)getAllFiles:(NSString *)root {
    NSFileManager* fm ;
    NSDirectoryEnumerator *dirEnumerator ;
    NSMutableArray *dirArray = [[NSMutableArray alloc]init] ;

    fm = [NSFileManager defaultManager];

    dirEnumerator = [fm enumeratorAtPath:root];
    NSString* file = [[NSString alloc]init];
    while( ( file = [dirEnumerator nextObject] ) != nil ) {
        [dirArray addObject:[root stringByAppendingPathComponent:file]];
    }
    return dirArray;
}

+ (NSString *)cgiEventConfigPath {
    return [[self projectRootDir] stringByAppendingPathComponent:@"/MMFoundation/EventService/CgiEventConfig.mm"];
}

+ (NSString *)generateMockCaseTemplateWithCgiModel:(MMCgiModel *)cgiModel {
    NSMutableSet *pathSet = [NSMutableSet set];
    for (NSString *nsPath in kProtofilePathsArray) {
        [pathSet addObject:[nsPath stringByDeletingLastPathComponent]];
    }
    NSString *cmdStr = @"./PB2JSON ";
    for (NSString *path in pathSet) {
        cmdStr = [cmdStr stringByAppendingString:[NSString stringWithFormat:@"--proto_path=%@ ", path]];
    }
    cmdStr = [cmdStr stringByAppendingString:[NSString stringWithFormat:@"--target=%@ ", cgiModel.messageName]];
    cmdStr = [cmdStr stringByAppendingString:[NSString stringWithFormat:@"--cgiNumber=%u ", cgiModel.cgiNumber]];
    cmdStr = [cmdStr stringByAppendingString:@"--isUpdateFromSvr=YES "];
    cmdStr = [cmdStr stringByAppendingString:[NSString stringWithFormat:@"--mockcase_out=%@ ", [[self projectRootDir] stringByAppendingPathComponent:@"tools/CgiMockTool/"]]];
    cmdStr = [cmdStr stringByAppendingString:[NSString stringWithFormat:@"%@", cgiModel.protoPath]];
    
    NSArray *cmdArray = [cmdStr componentsSeparatedByString:@" "];
    int argc = (int)cmdArray.count;
    char **argv = new char *[argc];
    for (int i = 0; i < argc; i++) {
        argv[i] = new char[200];
        NSString *nsStr = [cmdArray objectAtIndex:i];
        strcpy(argv[i], [nsStr cStringUsingEncoding:NSUTF8StringEncoding]);
        //NSLog(@"%s\n", argv[i]);
    }
    google::protobuf::compiler::CommandLineInterface cli;
    
    google::protobuf::compiler::objectivec::ObjectiveCGenerator objc_generator;
    cli.RegisterGenerator("--mockcase_out", &objc_generator, "Generate Mock Case Script File");
    
    int result = cli.Run(argc, argv);
    
    for (int i = 0; i < argc; i++) {
        delete[] argv[i];
    }
    delete[] argv;
    
    if (result != 0) {
        NSLog(@"error !!!!");
        return nil;
    }
    
    NSString *rootPath = [self workSpaceDir];
    
    NSArray *allFiles = [self getCurrentFiles:rootPath];
    for (NSString *path in allFiles) {
        if ([[path pathExtension] isEqualToString:@"js"]) {
            NSString *targetPath = [rootPath stringByAppendingPathComponent:path];
            NSError *error = nil;
            NSString *content = [NSString stringWithContentsOfFile:targetPath encoding:NSUTF8StringEncoding error:&error];
            [[NSFileManager defaultManager] removeItemAtPath:targetPath error:&error];
            
            return content;
        }
    }
    
    
    
    return nil;
}

+ (BOOL)saveMockCase:(MMCgiMockCase *)mockCase {
    if (mockCase == nil) {
        return NO;
    }
    
    NSString *mockcasePlistPath = [self getMockCasePlistPath];
    BOOL b;
    BOOL isFileExist = [[NSFileManager defaultManager] fileExistsAtPath:mockcasePlistPath isDirectory:&b];
    NSMutableDictionary *plistDict = nil;
    if (!isFileExist) {
        plistDict = [[NSMutableDictionary alloc] init];
    } else {
        NSDictionary *tempDict = [NSDictionary dictionaryWithContentsOfFile:mockcasePlistPath];
        if (tempDict == nil) {
            plistDict = [[NSMutableDictionary alloc] init];
        } else {
            plistDict = [tempDict mutableCopy];
        }
    }
    
    NSMutableArray *mockcaseArr = [[plistDict objectForKey:MOCKCASEARRAYKEY] mutableCopy];
    if (mockcaseArr == nil) {
        mockcaseArr = [[NSMutableArray alloc] init];
    }
    
    NSDictionary *targetDict = nil;
    for (NSDictionary *tempDict in mockcaseArr) {
        if ([[tempDict objectForKey:@"mockcaseId"] isEqualToString:mockCase.mockcaseId]) {
            targetDict = tempDict;
            break;
        }
    }
    
    if (targetDict != nil) {
        NSDictionary *tempDict = [mockCase mockcaseDict];
        [mockcaseArr replaceObjectAtIndex:[mockcaseArr indexOfObject:targetDict] withObject:tempDict];
    } else {
        [mockcaseArr addObject:[mockCase mockcaseDict]];
    }
    
    [plistDict setObject:mockcaseArr forKey:MOCKCASEARRAYKEY];
    
    return [plistDict writeToFile:mockcasePlistPath atomically:YES];
}

+ (NSMutableArray *)getMockCases {
    NSString *mockcasePlistPath = [self getMockCasePlistPath];
    BOOL b;
    BOOL isFileExist = [[NSFileManager defaultManager] fileExistsAtPath:mockcasePlistPath isDirectory:&b];
    if (!isFileExist) {
        return nil;
    } else {
        NSDictionary *tempDict = [NSDictionary dictionaryWithContentsOfFile:mockcasePlistPath];
        if (tempDict == nil) {
            return nil;
        } else {
            return [[tempDict objectForKey:MOCKCASEARRAYKEY] mutableCopy];
        }
    }
}

+ (BOOL)createMockCase:(NSString *)mockCaseName {
    
    if (mockCaseName.length == 0) {
        return NO;
    }
    
    NSString *mockcasePlistPath = [self getMockCasePlistPath];
    BOOL b;
    BOOL isFileExist = [[NSFileManager defaultManager] fileExistsAtPath:mockcasePlistPath isDirectory:&b];
    NSMutableDictionary *plistDict = nil;
    if (!isFileExist) {
        plistDict = [[NSMutableDictionary alloc] init];
    } else {
        NSDictionary *tempDict = [NSDictionary dictionaryWithContentsOfFile:mockcasePlistPath];
        if (tempDict == nil) {
            plistDict = [[NSMutableDictionary alloc] init];
        } else {
            plistDict = [tempDict mutableCopy];
        }
    }
    
    NSMutableArray *mockcaseArr = [[plistDict objectForKey:MOCKCASEARRAYKEY] mutableCopy];
    if (mockcaseArr == nil) {
        mockcaseArr = [[NSMutableArray alloc] init];
    }
    
    NSString *mocaseid = [NSUUID UUID].UUIDString;
    NSDictionary *mockcaseDict = @{@"mockCaseName":mockCaseName,  @"mockcaseid":mocaseid};
    [mockcaseArr addObject:mockcaseDict];
    [plistDict setObject:mockcaseArr forKey:MOCKCASEARRAYKEY];
    
    return [plistDict writeToFile:mockcasePlistPath atomically:YES];
}

+ (NSString *)getMockCasePlistPath {
    NSString *rootPath = [self workSpaceDir];
    NSString *mockcasePath = [rootPath stringByAppendingPathComponent:MOCKCASEPLIST];
    return mockcasePath;
}

+ (void)showAlert:(NSWindow *)window title:(NSString *)title message:(NSString *)message{
    NSAlert *alert = [NSAlert new];
    [alert addButtonWithTitle:@"确定"];
    [alert setMessageText:title.length > 0 ?  title : @""];
    [alert setInformativeText:message.length > 0 ? message : @""];
    [alert setAlertStyle:NSAlertStyleWarning];
    [alert beginSheetModalForWindow:window completionHandler:nil];
}

+ (BOOL)removeMockCase:(MMCgiMockCase *)mockCase {
    if (mockCase == nil) {
        return NO;
    }
    
    NSString *mockcasePlistPath = [self getMockCasePlistPath];
    BOOL b;
    BOOL isFileExist = [[NSFileManager defaultManager] fileExistsAtPath:mockcasePlistPath isDirectory:&b];
    NSMutableDictionary *plistDict = nil;
    if (!isFileExist) {
        return NO;
    } else {
        NSDictionary *tempDict = [NSDictionary dictionaryWithContentsOfFile:mockcasePlistPath];
        if (tempDict == nil) {
            return NO;
        } else {
            plistDict = [tempDict mutableCopy];
        }
    }
    NSMutableArray *mockcaseArr = [[plistDict objectForKey:MOCKCASEARRAYKEY] mutableCopy];
    if (mockcaseArr == nil) {
        return NO;
    }
    
    NSDictionary *targetDict = nil;
    for (NSDictionary *tempDict in mockcaseArr) {
        if ([[tempDict objectForKey:@"mockcaseId"] isEqualToString:[mockCase mockcaseId]]) {
            targetDict = tempDict;
            break;
        }
    }
    
    if (targetDict != nil) {
        [mockcaseArr removeObject:targetDict];
        [plistDict setObject:mockcaseArr forKey:MOCKCASEARRAYKEY];
        
        return [plistDict writeToFile:mockcasePlistPath atomically:YES];
    }
    return NO;
}

@end
