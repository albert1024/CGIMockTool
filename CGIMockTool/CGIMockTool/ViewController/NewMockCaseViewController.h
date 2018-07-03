//
//  NewMockCaseViewController.h
//  CGIMockTool
//
//  Created by littleliang on 2018/6/27.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class MMCgiMockCase;
@protocol NewMockCaseViewControllerDelegate <NSObject>

- (void)onNewMockCaseViewControllerCreateMockCase;

- (void)onNewMockCaseViewControllerEditMockCase;
@end

typedef NS_ENUM(NSUInteger, NewMockCaseViewControllerType) {
    NewMockCaseViewControllerTypeNew,
    NewMockCaseViewControllerTypeEdit,
};

@interface NewMockCaseViewController : NSViewController

- (void)setMockCase:(MMCgiMockCase *)mockcase;

@property(nonatomic, assign) NewMockCaseViewControllerType controllerType;

@property(nonatomic, weak) id<NewMockCaseViewControllerDelegate> delegate;



@end
