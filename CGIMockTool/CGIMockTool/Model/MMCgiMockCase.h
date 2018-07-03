//
//  MMCgiMockCase.h
//  CGIMockTool
//
//  Created by littleliang on 2018/6/26.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import <Foundation/Foundation.h>

@class MMCgiMockScript;

@interface MMCgiMockCase : NSObject

@property(nonatomic, strong) NSMutableArray *cgiMockScripts;
@property(nonatomic, strong) NSString *mockcaseName;
@property(nonatomic, strong) NSString *mockcaseId;

- (instancetype)initWithDictionary:(NSDictionary *)dict;
- (instancetype)initWithMockCaseName:(NSString *)mockcaseName;
- (NSDictionary *)mockcaseDict;

@end
