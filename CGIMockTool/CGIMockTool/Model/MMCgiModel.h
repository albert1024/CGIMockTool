//
//  MMCgiModel.h
//  CGIMockTool
//
//  Created by littleliang on 2018/6/11.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface MMCgiModel : NSObject<NSCoding>

@property(nonatomic, strong) NSString *cgiFuncName;
@property(nonatomic, assign) UInt32 cgiNumber;
@property(nonatomic, strong) NSString *messageName;
@property(nonatomic, strong) NSString *protoPath;
@property(nonatomic, assign) BOOL isCrossPlatform;

@end
