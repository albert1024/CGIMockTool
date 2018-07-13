//
//  MMCgiMockCase.m
//  CGIMockTool
//
//  Created by littleliang on 2018/6/26.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import "MMCgiMockCase.h"

@implementation MMCgiMockCase

- (instancetype)initWithDictionary:(NSDictionary *)dict {
    self = [super init];
    if (self) {
        _mockcaseId = [dict objectForKey:@"mockcaseId"];
        _mockcaseName = [dict objectForKey:@"mockcaseName"];
        _cgiMockScripts = [dict objectForKey:@"cgiMockScripts"];
        _cgiMockScriptsSelectDict = [dict objectForKey:@"cgiMockScriptsSelectDict"];
    }
    return self;
}

- (instancetype)initWithMockCaseName:(NSString *)mockcaseName {
    self = [super init];
    if (self) {
        _mockcaseId = [NSUUID UUID].UUIDString;
        _mockcaseName = mockcaseName;
        _cgiMockScripts = [[NSMutableArray alloc] init];
        _cgiMockScriptsSelectDict = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (NSDictionary *)mockcaseDict {
    NSMutableDictionary *mDict = [[NSMutableDictionary alloc] init];
    [mDict setObject:_mockcaseId forKey:@"mockcaseId"];
    [mDict setObject:_mockcaseName forKey:@"mockcaseName"];
    [mDict setObject:_cgiMockScripts forKey:@"cgiMockScripts"];
    [mDict setObject:_cgiMockScriptsSelectDict forKey:@"cgiMockScriptsSelectDict"];
    return [mDict copy];
}

@end
