//
//  NewMockCaseViewController.m
//  CGIMockTool
//
//  Created by littleliang on 2018/6/27.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import "NewMockCaseViewController.h"
#import "MMMockToolUtil.h"
#import "MMCgiMockCase.h"
@interface NewMockCaseViewController ()<NSTableViewDelegate, NSTableViewDataSource, NSTextFieldDelegate>

@property (weak) IBOutlet NSButton *createButton;

@property (weak) IBOutlet NSTextField *mockcaseTextField;

@property(nonatomic, strong) MMCgiMockCase *mockcase;

@end

@implementation NewMockCaseViewController

- (void)setMockCase:(MMCgiMockCase *)mockcase {
    _mockcase = mockcase;
    self.mockcaseTextField.stringValue = self.mockcase.mockcaseName;
    if (self.mockcaseTextField.stringValue.length == 0) {
        self.createButton.enabled = NO;
    } else {
        self.createButton.enabled = YES;
    }
}

- (void)setControllerType:(NewMockCaseViewControllerType)controllerType {
    _controllerType = controllerType;
    if (_controllerType == NewMockCaseViewControllerTypeNew) {
        self.mockcase = nil;
        self.title = @"创建用例";
        [self.createButton setTitle:@"创建"];
    } else {
        self.title = @"修改";
        [self.createButton setTitle:@"修改"];
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
    _mockcaseTextField.delegate = self;
    if (self.controllerType == NewMockCaseViewControllerTypeNew) {
        self.mockcase = nil;
        self.title = @"创建用例";
        [self.createButton setTitle:@"创建"];
    } else {
        self.mockcaseTextField.stringValue = self.mockcase.mockcaseName;
        [self.createButton setTitle:@"修改"];
    }
    
    if (self.mockcaseTextField.stringValue.length == 0) {
        self.createButton.enabled = NO;
    }
}

- (void)controlTextDidChange:(NSNotification *)notification {
    NSTextField *textField = [notification object];
    
    if (textField.stringValue.length > 0) {
        self.createButton.enabled = YES;
    } else {
        self.createButton.enabled = NO;
    }
}

- (IBAction)onButtonClick:(id)sender {
    if (self.controllerType == NewMockCaseViewControllerTypeNew) {
        MMCgiMockCase *tempMockCase = [[MMCgiMockCase alloc] initWithMockCaseName:self.mockcaseTextField.stringValue];
        [MMMockToolUtil saveMockCase:tempMockCase];
        if ([self.delegate respondsToSelector:@selector(onNewMockCaseViewControllerCreateMockCase)]) {
            [self.delegate onNewMockCaseViewControllerCreateMockCase];
            [self dismissViewController:self];
        }
    } else {
        self.mockcase.mockcaseName = self.mockcaseTextField.stringValue;
        [MMMockToolUtil saveMockCase:self.mockcase];
        if ([self.delegate respondsToSelector:@selector(onNewMockCaseViewControllerEditMockCase)]) {
            [self.delegate onNewMockCaseViewControllerEditMockCase];
            [self dismissViewController:self];
        }
    }
}


@end
