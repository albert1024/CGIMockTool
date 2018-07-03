//
//  JSONEditorViewController.h
//  CGIMockTool
//
//  Created by littleliang on 2018/6/15.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class MMCgiMockScript;

typedef NS_ENUM(NSUInteger, JSONEditorViewControllerType) {
    JSONEditorViewControllerTypeNew,
    JSONEditorViewControllerTypeEdit,
};

@interface JSONEditorViewController : NSViewController

- (void)setCgiMockScript:(MMCgiMockScript *)mockScript;

@property(nonatomic, assign) JSONEditorViewControllerType controllerType;

@end
