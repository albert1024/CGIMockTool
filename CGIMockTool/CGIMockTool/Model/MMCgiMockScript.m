//
//  MMCgiMockCase.m
//  CGIMockTool
//
//  Created by littleliang on 2018/6/22.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import "MMCgiMockScript.h"

@implementation MMCgiMockScript

- (void)encodeWithCoder:(NSCoder *)aCoder {
    [aCoder encodeObject:self.cgiModel forKey:@"cgiModel"];
    [aCoder encodeObject:self.jsonScript forKey:@"jsonScript"];
    [aCoder encodeObject:self.scriptRemark forKey:@"scriptRemark"];
}

- (nullable instancetype)initWithCoder:(NSCoder *)aDecoder {
    self = [super init];
    if (self) {
        _cgiModel = [aDecoder decodeObjectForKey:@"cgiModel"];
        _jsonScript = [aDecoder decodeObjectForKey:@"jsonScript"];
        _scriptRemark = [aDecoder decodeObjectForKey:@"scriptRemark"];
    }
    return self;
}

@end
