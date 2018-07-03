//
//  MMCgiMockCase.h
//  CGIMockTool
//
//  Created by littleliang on 2018/6/22.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MMCgiModel.h"

@interface MMCgiMockScript : NSObject <NSCoding>

@property(nonatomic, strong) MMCgiModel *cgiModel;
@property(nonatomic, strong) NSString *scriptName;
@property(nonatomic, strong) NSString *jsonScript;
@property(nonatomic, strong) NSString *scriptRemark;
@property(nonatomic, assign) BOOL isUpdateFromSvr;
@property(nonatomic, strong) NSString *scriptDirectory;

@end
