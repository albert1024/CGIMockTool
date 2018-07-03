//
//  MockCaseMangeViewController.m
//  CGIMockTool
//
//  Created by littleliang on 2018/6/27.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import "MockCaseMangeViewController.h"
#import "NewMockCaseWindowController.h"
#import "NewMockCaseViewController.h"
#import "MMMockToolUtil.h"
#import "MMCgiMockCase.h"
#import "JSONEditorWindowViewController.h"
#import "JSONEditorViewController.h"
#import "MMCgiMockScript.h"

@interface MockCaseMangeViewController ()<NSTabViewDelegate, NSTableViewDataSource, NewMockCaseViewControllerDelegate>
@property (weak) IBOutlet NSTableView *mockCaseTableView;
@property (weak) IBOutlet NSTableView *mockScriptsTableView;
@property (weak) IBOutlet NSTextField *errorLabel;

@property(nonatomic, strong) NSMutableArray<MMCgiMockCase *> *mockcaseDataArr;

@property(nonatomic, strong) MMCgiMockCase *selectedMockCase;

@property(nonatomic, strong) NSString *selectedMockScriptPath;

@property (weak) IBOutlet NSSearchField *searchTextField;

@property(nonatomic, strong) NSMutableArray<MMCgiMockCase *> *searchMockCaseArray;

@end

@implementation MockCaseMangeViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
    self.title = @"Mock Case Management";
    
    self.mockcaseDataArr = [[NSMutableArray alloc] init];
    
    self.mockCaseTableView.delegate = self;
    self.mockCaseTableView.dataSource = self;
    
    self.mockScriptsTableView.delegate = self;
    self.mockScriptsTableView.dataSource = self;
    
    self.searchTextField.delegate = self;
    
    [self loadMockCase];
    [self.mockCaseTableView reloadData];
    [self.mockCaseTableView setDoubleAction:@selector(onDoubleClickMockCaseCell:)];
    [self.mockScriptsTableView setDoubleAction:@selector(onDoubleClickMockScriptCell:)];
    self.errorLabel.stringValue = @"";
    
    self.searchMockCaseArray = [[NSMutableArray alloc] init];
}

- (void)onDoubleClickMockCaseCell:(id)sender {
    NSInteger rowNumber = [self.mockCaseTableView clickedRow];
    self.selectedMockCase = [self.mockcaseDataArr objectAtIndex:rowNumber];
    NewMockCaseWindowController *wvc = [self.storyboard instantiateControllerWithIdentifier:@"NewMockCaseWindowController"];
    NewMockCaseViewController *vc = (NewMockCaseViewController *)wvc.contentViewController;
    vc.controllerType = NewMockCaseViewControllerTypeEdit;
    vc.delegate = self;
    [vc setMockCase:self.selectedMockCase];
    [self presentViewControllerAsModalWindow:[wvc contentViewController]];
}

- (void)controlTextDidChange:(NSNotification *)notification {
    [self.mockScriptsTableView deselectAll:nil];
    [self.mockScriptsTableView deselectAll:nil];
    NSSearchField *searchField = [notification object];
    NSString *searchStr = [[searchField stringValue] lowercaseString];
    [self.searchMockCaseArray removeAllObjects];
    for (MMCgiMockCase *mockCase in self.mockcaseDataArr) {
        NSString *mockCaseStr = [mockCase.mockcaseName lowercaseString];
        if ([mockCaseStr containsString:searchStr]) {
            [self.searchMockCaseArray addObject:mockCase];
        }
    }
    self.selectedMockCase = nil;
    self.selectedMockScriptPath = nil;
    [self.mockCaseTableView reloadData];
    [self.mockScriptsTableView reloadData];
}

- (void)onDoubleClickMockScriptCell:(id)sender {
    NSInteger rowNumber = [self.mockScriptsTableView clickedRow];
    self.selectedMockScriptPath = [self.selectedMockCase.cgiMockScripts objectAtIndex:rowNumber];
    
    JSONEditorWindowViewController *wvc = [self.storyboard instantiateControllerWithIdentifier:@"JSONEditorWindowViewController"];
    
    JSONEditorViewController *contentViewController = (JSONEditorViewController *)[wvc contentViewController];
    contentViewController.controllerType = JSONEditorViewControllerTypeEdit;
    MMCgiMockScript *mockScript = [self getMockScriptByScriptPath:self.selectedMockScriptPath];
    [contentViewController setCgiMockScript:mockScript];
    [self presentViewControllerAsModalWindow:contentViewController];
}

- (MMCgiMockScript *)getMockScriptByScriptPath:(NSString *)scriptPath {
    NSString *mockcaseRootDir = [MMMockToolUtil mockcaseRootDir];
    NSString *mockcaseAbsoulutePath = [mockcaseRootDir stringByAppendingPathComponent:scriptPath];
    
    NSError *error = nil;
    
    NSString *fileContent = [NSString stringWithContentsOfFile:mockcaseAbsoulutePath encoding:NSUTF8StringEncoding error:&error];
    NSArray *lines = [fileContent componentsSeparatedByString:@"\n"];
    NSEnumerator *enumator = [lines objectEnumerator];
    NSString *tmp = nil;
    NSString *nsMockScripts = @"{\n";
    tmp = [enumator nextObject];
    while (tmp = [enumator nextObject]) {
        if ([tmp containsString:@"mockRequest("]) {
            //NSLog(@"%@", tmp);
            tmp = [tmp stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
            NSArray *components = [tmp componentsSeparatedByString:@"."];
            NSString *componet1 = [components objectAtIndex:0];
            
            NSRange range1 = [componet1 rangeOfString:@"("];
            NSRange range2 = [componet1 rangeOfString:@")"];
            NSMakeRange(range1.location + 1, range2.location - range2.location);
            NSString *cgiNumberStr = [componet1 substringWithRange:NSMakeRange(range1.location + 1, range2.location - range1.location - 1)];
            
            NSString *componet2 = [components objectAtIndex:1];
            range1 = [componet2 rangeOfString:@"("];
            range2 = [componet2 rangeOfString:@")"];
            
            NSString *isUpdateFromSvrStr = [componet2 substringWithRange:NSMakeRange(range1.location + 1, range2.location - range1.location - 1)];
            
            BOOL bIsUpdateFromSvr = [isUpdateFromSvrStr isEqualToString:@"YES"] ? YES : NO;
            UInt32 uCgiNumber = (UInt32)[cgiNumberStr integerValue];
            
            MMCgiMockScript *mockScript = [[MMCgiMockScript alloc] init];
            MMCgiModel *model = [[MMCgiModel alloc] init];
            model.cgiNumber = uCgiNumber;
            mockScript.cgiModel = model;
            mockScript.isUpdateFromSvr = bIsUpdateFromSvr;
            mockScript.jsonScript = nsMockScripts;
            mockScript.scriptName = [self.selectedMockScriptPath lastPathComponent];
            
            mockScript.scriptDirectory = [self.selectedMockScriptPath stringByDeletingLastPathComponent];
            
            return mockScript;
        } else {
            nsMockScripts = [[nsMockScripts stringByAppendingString:tmp] stringByAppendingString:@"\n"];
        }
    }
    return  nil;
}


- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    if (tableView == self.mockCaseTableView) {
        if (self.searchTextField.stringValue.length > 0) {
            return self.searchMockCaseArray.count;
        }
        return self.mockcaseDataArr.count;
    } else {
        return self.selectedMockCase.cgiMockScripts.count;
    }
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    if (tableView == self.mockCaseTableView) {
        if (self.searchTextField.stringValue.length > 0) {
            MMCgiMockCase *mockcase = [self.searchMockCaseArray objectAtIndex:row];
            return [NSString stringWithFormat:@"%@", mockcase.mockcaseName];
        }
        MMCgiMockCase *mockcase = [self.mockcaseDataArr objectAtIndex:row];
        return [NSString stringWithFormat:@"%@", mockcase.mockcaseName];
    } else {
        NSString *scriptPath = [self.selectedMockCase.cgiMockScripts objectAtIndex:row];
        return scriptPath;
    }
}

- (void)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row {
    if (tableView == self.mockCaseTableView) {
        if (self.searchTextField.stringValue.length > 0) {
            self.selectedMockCase = [self.searchMockCaseArray objectAtIndex:row];
            [self.mockScriptsTableView reloadData];
        } else {
            self.selectedMockCase = [self.mockcaseDataArr objectAtIndex:row];
            [self.mockScriptsTableView reloadData];
        }
    } else {
        self.selectedMockScriptPath = [self.selectedMockCase.cgiMockScripts objectAtIndex:row];
    }
}

- (IBAction)onClickCreateMockCase:(id)sender {
    NewMockCaseWindowController *wvc = [self.storyboard instantiateControllerWithIdentifier:@"NewMockCaseWindowController"];
    NewMockCaseViewController *vc = (NewMockCaseViewController *)wvc.contentViewController;
    vc.controllerType = NewMockCaseViewControllerTypeNew;
    vc.delegate = self;
    [self presentViewControllerAsModalWindow:[wvc contentViewController]];
}

- (void)onNewMockCaseViewControllerCreateMockCase {
    [self loadMockCase];
    [self.mockCaseTableView reloadData];
}

- (void)loadMockCase {
    [self.mockcaseDataArr removeAllObjects];
    for (NSDictionary *tempDict in [MMMockToolUtil getMockCases]) {
        [self.mockcaseDataArr addObject:[[MMCgiMockCase alloc] initWithDictionary:tempDict]];
    }
}

- (void)onNewMockCaseViewControllerEditMockCase {
    [self loadMockCase];
    [self.mockCaseTableView reloadData];
}

- (IBAction)addMockScript:(id)sender {
    
    if (self.selectedMockCase == nil) {
        [MMMockToolUtil showAlert:self.view.window title:@"" message:@"请选择一个用例"];
        return;
    }
    
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setDirectoryURL:[NSURL URLWithString:[NSString stringWithFormat:@"%@/MMTest/CgiMock/CgiMockTestCase/", [MMMockToolUtil projectRootDir]]]];
    panel.canChooseDirectories = NO;
    panel.allowsMultipleSelection = YES;
    
    NSMutableArray *selectedArray = [[NSMutableArray alloc] init];
    if ([panel runModal] == NSModalResponseOK) {
        NSArray *urls = [panel URLs];
        for (NSURL *fileUrl in urls) {
            NSString *nsFilePath = [fileUrl absoluteString];
            NSLog(@"%@", nsFilePath);
            NSArray *components = [nsFilePath componentsSeparatedByString:@"/"];
            NSString *requireString = @"";
            for (int i = 0; i < components.count; i++) {
                NSString *component = [components objectAtIndex:i];
                if ([component isEqualToString:@"CgiMockTestCase"]) {
                    for (int j = i + 1; j < components.count; j++) {
                        requireString = [requireString stringByAppendingPathComponent:[components objectAtIndex:j]];
                    }
                    break;
                }
            }
            if (requireString.length > 0) {
                [selectedArray addObject:requireString];
            }
        }
    } else {
        NSLog(@"Click Cancel.");
    }
    NSMutableArray *cgiMockScripts = nil;
    if (self.selectedMockCase.cgiMockScripts == nil) {
        cgiMockScripts = [[NSMutableArray alloc] init];
    } else {
        cgiMockScripts = [self.selectedMockCase.cgiMockScripts mutableCopy];
    }
    [cgiMockScripts addObjectsFromArray:selectedArray];
    self.selectedMockCase.cgiMockScripts = cgiMockScripts;
    [MMMockToolUtil saveMockCase:self.selectedMockCase];
    [self.mockScriptsTableView reloadData];
}

- (IBAction)removeMockScript:(id)sender {
    if (self.selectedMockCase == nil || self.selectedMockScriptPath == nil) {
        [MMMockToolUtil showAlert:self.view.window title:@"" message:@"请选择一个脚本"];
        return;
    }
    NSMutableArray *tempScripts = [self.selectedMockCase.cgiMockScripts mutableCopy];
    [tempScripts removeObject:self.selectedMockScriptPath];
    self.selectedMockCase.cgiMockScripts = tempScripts;
    
    [MMMockToolUtil saveMockCase:self.selectedMockCase];;
    [self.mockScriptsTableView reloadData];
}

- (IBAction)onApplyBtnClick:(id)sender {
    if (self.selectedMockCase == nil) {
        [MMMockToolUtil showAlert:self.view.window title:@"" message:@"请选择一个用例"];
        return;
    }
    NSArray *selectedMockCaseScripts = [self.selectedMockCase.cgiMockScripts copy];
    NSString *outputCgiMockStageFile = [NSString stringWithFormat:@"//Mock case name : %@\n", self.selectedMockCase.mockcaseName];
    for (NSString *mockScript in selectedMockCaseScripts) {
        NSString *tempStr = [NSString stringWithFormat:@"require(\"%@\")\n", mockScript];
        outputCgiMockStageFile = [outputCgiMockStageFile stringByAppendingString:tempStr];
    }
    NSString *filePath = [[MMMockToolUtil projectRootDir] stringByAppendingPathComponent:@"MMTest/CgiMock/CgiMockTestCase/cgimockstage.js"];
    NSError *error = nil;
    [outputCgiMockStageFile writeToFile:filePath atomically:YES encoding:NSUTF8StringEncoding error:&error];
    if (error) {
        [MMMockToolUtil showAlert:self.view.window title:@"" message:error.description];
        return;
    }
    [MMMockToolUtil showAlert:self.view.window title:@"" message:@"设置Mock Case成功"];
}

- (IBAction)onRemoveBtnClick:(id)sender {
    if (self.selectedMockCase == nil) {
        [MMMockToolUtil showAlert:self.view.window title:@"" message:@"请选择一个用例"];
        return;
    }
    
    [MMMockToolUtil removeMockCase:self.selectedMockCase];
    self.selectedMockCase = nil;
    [self loadMockCase];
    [self.mockCaseTableView reloadData];
    [self.mockScriptsTableView reloadData];
}

@end
