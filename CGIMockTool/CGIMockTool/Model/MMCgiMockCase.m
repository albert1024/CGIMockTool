//
//  MMCgiMockCase.m
//  CGIMockTool
//
//  Created by littleliang on 2018/6/26.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import "MMCgiMockCase.h"

@implementation MMCgiMockCase

//- (void)encodeWithCoder:(NSCoder *)aCoder {
//    [aCoder encodeObject:self.cgiMockScripts forKey:@"cgiMockScripts"];
//    [aCoder encodeObject:self.mockCaseName forKey:@"mockCaseName"];
//}
//
//- (nullable instancetype)initWithCoder:(NSCoder *)aDecoder {
//    self = [super init];
//    if (self) {
//        _cgiMockScripts = [aDecoder decodeObjectForKey:@"cgiMockScripts"];
//        _mockCaseName = [aDecoder decodeObjectForKey:@"mockCaseName"];
//    }
//    return self;
//}

- (instancetype)initWithDictionary:(NSDictionary *)dict {
    self = [super init];
    if (self) {
        _mockcaseId = [dict objectForKey:@"mockcaseId"];
        _mockcaseName = [dict objectForKey:@"mockcaseName"];
        _cgiMockScripts = [dict objectForKey:@"cgiMockScripts"];
    }
    return self;
}

- (instancetype)initWithMockCaseName:(NSString *)mockcaseName {
    self = [super init];
    if (self) {
        _mockcaseId = [NSUUID UUID].UUIDString;
        _mockcaseName = mockcaseName;
        _cgiMockScripts = [[NSMutableArray alloc] init];
    }
    return self;
}

- (NSDictionary *)mockcaseDict {
    NSMutableDictionary *mDict = [[NSMutableDictionary alloc] init];
    [mDict setObject:_mockcaseId forKey:@"mockcaseId"];
    [mDict setObject:_mockcaseName forKey:@"mockcaseName"];
    [mDict setObject:_cgiMockScripts forKey:@"cgiMockScripts"];
    return [mDict copy];
}

@end
