//
//  RootViewController.m
//  CGIMockTool
//
//  Created by littleliang on 2018/6/9.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import "RootViewController.h"
#import "MMMockToolUtil.h"
#import "SetRootDirViewController.h"
#import "SetRootDirWindowViewController.h"
#import "NSView+Extend.h"
#import "MMCgiModel.h"
#import "JSONEditorWindowViewController.h"
#import "JSONEditorViewController.h"
#import "MMCgiMockScript.h"
#import "MockCaseMangeWindowController.h"
#import "MMSearchTextField.h"

@interface RootViewController()<NSTabViewDelegate, NSTableViewDataSource, NSSearchFieldDelegate>

@property (weak) IBOutlet NSTableView *cgiNameTableView;
@property(nonatomic, strong) NSMutableArray *cgiModelArray;
@property (weak) IBOutlet MMSearchTextField *searchField;
@property (weak) IBOutlet NSTextField *errorLabel;

@property(nonatomic, strong) MMCgiModel *selectedCgiModel;

@property(nonatomic, strong) NSMutableArray *searchCgiModelArray;

@property(nonatomic, strong) NSString *m_generateContent;

@end

@implementation RootViewController



- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.
    //[self checkAndSetRootDir];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onReceiveSaveRootDirNotification) name:kNotificationSaveRootDir object:nil];
    [self setupData];
    self.errorLabel.stringValue = @"";
}

- (void)setupData {
    self.searchCgiModelArray = [[NSMutableArray alloc] init];
    if ([MMMockToolUtil projectRootDir].length == 0){
        return;
    }
    
    if (self.cgiModelArray.count > 0) {
        return;
    }
    
    [self reloadData];
}

- (void)reloadData {
    NSArray *modelArray = [MMMockToolUtil parseProtoFilesToCgiModel:[MMMockToolUtil protoFiles]];
    NSArray *crossPlatformArray = [MMMockToolUtil parseCrossplatformProtoFilesToCgiModel];
    self.cgiModelArray = [[NSMutableArray alloc] init];
    [self.cgiModelArray addObjectsFromArray:modelArray];
    [self.cgiModelArray addObjectsFromArray:crossPlatformArray];
    [self.cgiNameTableView reloadData];
}

- (void)onReceiveSaveRootDirNotification {
    [self refreshView];
}

- (void)refreshView {
    if ([MMMockToolUtil projectRootDir].length == 0) {
        [self.view hideAllSubviews];
        return;
    }
    //cgiNameTableView
    [self.view showAllSubviews];
    [self setupData];
    self.cgiNameTableView.delegate = self;
    self.cgiNameTableView.dataSource = self;
    
    self.searchField.delegate = self;
}

- (void)controlTextDidChange:(NSNotification *)notification {
    [self.cgiNameTableView deselectAll:nil];
    [self.searchCgiModelArray removeAllObjects];
    NSSearchField *searchField = [notification object];
    NSString *searchString = searchField.stringValue;
    UInt32 cgiNum = (UInt32)[searchString integerValue];
    if (cgiNum > 0) {
        NSString *nsSearchFieldCgiNum = [NSString stringWithFormat:@"%u", cgiNum];
        for (MMCgiModel *tempModel in self.cgiModelArray) {
            NSString *nsModelCgiNum = [NSString stringWithFormat:@"%u", tempModel.cgiNumber];
            if ([nsModelCgiNum containsString:nsSearchFieldCgiNum]) {
                [self.searchCgiModelArray addObject:tempModel];
            }
        }
    } else {
        for (MMCgiModel *tempModel in self.cgiModelArray) {
            if ([[tempModel.cgiFuncName lowercaseString] containsString:[searchString lowercaseString]]) {
                [self.searchCgiModelArray addObject:tempModel];
            }
        }
    }
    
    BOOL isFound = NO;
    for (MMCgiModel *tempModel in self.searchCgiModelArray) {
        if (self.selectedCgiModel.cgiNumber == tempModel.cgiNumber) {
            isFound = YES;
            break;
        }
    }
    if (!isFound) {
        self.selectedCgiModel = nil;
    }
    [self.cgiNameTableView reloadData];
}

- (void)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row {
    if (self.searchField.stringValue.length > 0) {
        self.selectedCgiModel = [self.searchCgiModelArray objectAtIndex:row];
    } else {
        self.selectedCgiModel = [self.cgiModelArray objectAtIndex:row];
    }
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return self.searchField.stringValue.length > 0 ? [self.searchCgiModelArray count] : [self.cgiModelArray count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    MMCgiModel *tempModel = self.searchField.stringValue.length > 0 ? [self.searchCgiModelArray objectAtIndex:row] : [self.cgiModelArray objectAtIndex:row];
    return [NSString stringWithFormat:@"%u %@", tempModel.cgiNumber, tempModel.cgiFuncName];
}
     
- (void)dealloc {
 
}

- (void)viewDidAppear {
    [super viewDidAppear];
    //[self checkAndSetRootDir];
    [self refreshView];
}

- (void)viewWillLayout {
    [super viewWillLayout];
    
}

- (void)checkAndSetRootDir {
    if ([MMMockToolUtil projectRootDir].length > 0) {
        return;
    }
    SetRootDirWindowViewController *setRootrDirViewController = [self.storyboard instantiateControllerWithIdentifier:@"SetRootDirWindowViewController"];
    [self presentViewControllerAsModalWindow:setRootrDirViewController.contentViewController];
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (IBAction)onClickJSONEditor:(id)sender {
    
    if (self.selectedCgiModel) {
        self.errorLabel.stringValue = @"";
        NSString *jsContent = [MMMockToolUtil generateMockCaseTemplateWithCgiModel:self.selectedCgiModel];
        MMCgiMockScript *mockScript = [[MMCgiMockScript alloc] init];
        mockScript.jsonScript = jsContent;
        mockScript.cgiModel = self.selectedCgiModel;
        JSONEditorWindowViewController *jsonEditorWindowViewController = [self.storyboard instantiateControllerWithIdentifier:@"JSONEditorWindowViewController"];
        JSONEditorViewController *contentViewController = (JSONEditorViewController *)[jsonEditorWindowViewController contentViewController];
        [contentViewController setCgiMockScript:mockScript];
        [self presentViewControllerAsModalWindow:jsonEditorWindowViewController.contentViewController];
        
    } else {
        self.errorLabel.stringValue = @"请选择一个Cgi";
    }
}

- (IBAction)onClickMangeMockCaseBtn:(id)sender {
    MockCaseMangeWindowController *vc = [self.storyboard instantiateControllerWithIdentifier:@"MockCaseMangeWindowController"];
    [self presentViewControllerAsModalWindow:vc.contentViewController];
}

@end
