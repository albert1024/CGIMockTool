//
//  SetRootDirViewController.m
//  CGIMockTool
//
//  Created by littleliang on 2018/6/10.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import "SetRootDirViewController.h"
#import "MMMockToolUtil.h"

@interface SetRootDirViewController ()

@property (weak) IBOutlet NSTextField *rootDirTextField;

@property (weak) IBOutlet NSTextField *errorLabel;



@end

@implementation SetRootDirViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
    self.title = @"设置项目地址";
    [self setupTextFieldContent];
}

- (void)setupTextFieldContent {
    if ([MMMockToolUtil projectRootDir].length > 0) {
        _rootDirTextField.stringValue = [MMMockToolUtil projectRootDir];
        return;
    }
    
    NSString *bundlePath = [[NSBundle mainBundle] bundlePath];
    NSString *defaultRootPath = [[[bundlePath stringByDeletingLastPathComponent] stringByDeletingLastPathComponent] stringByDeletingLastPathComponent];
//    NSLog(@"%@", defaultRootPath);
    
    self.rootDirTextField.stringValue = defaultRootPath;
}

- (IBAction)onClickConfirmBtn:(id)sender {
    NSString *projectRootDir = self.rootDirTextField.stringValue;
    if (projectRootDir.length == 0) {
        self.errorLabel.stringValue = @"请输入项目根目录";
        return;
    }
    self.errorLabel.stringValue = @"";
    [MMMockToolUtil saveProjectRootDir:projectRootDir];
    [self dismissViewController:self];
    [[NSNotificationCenter defaultCenter] postNotificationName:kNotificationSaveRootDir object:nil];
}


@end
