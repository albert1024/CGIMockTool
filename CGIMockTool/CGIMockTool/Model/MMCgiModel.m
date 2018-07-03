//
//  MMCgiModel.m
//  CGIMockTool
//
//  Created by littleliang on 2018/6/11.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import "MMCgiModel.h"

@implementation MMCgiModel

- (NSString *)description {
    return [NSString stringWithFormat:@"%@ %u %@ %@", self.cgiFuncName, self.cgiNumber, self.messageName, self.protoPath];
}

- (void)encodeWithCoder:(NSCoder *)aCoder {
    [aCoder encodeObject:self.cgiFuncName forKey:@"cgiFuncName"];
    [aCoder encodeInteger:self.cgiNumber forKey:@"cgiNumber"];
    [aCoder encodeObject:self.messageName forKey:@"messageName"];
    [aCoder encodeObject:self.protoPath forKey:@"protoPath"];
}

- (nullable instancetype)initWithCoder:(NSCoder *)aDecoder {
    self = [super init];
    if (self) {
        self.cgiFuncName = [aDecoder decodeObjectForKey:@"cgiFuncName"];
        self.cgiNumber = (UInt32)[aDecoder decodeIntegerForKey:@"cgiNumber"];
        self.messageName = [aDecoder decodeObjectForKey:@"messageName"];
        self.protoPath = [aDecoder decodeObjectForKey:@"protoPath"];
    }
    return self;
}


@end
