//
//  JSONEditorViewController.m
//  CGIMockTool
//
//  Created by littleliang on 2018/6/15.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import "JSONEditorViewController.h"
#import <WebKit/WebKit.h>
#import "MMCgiMockScript.h"
#import "MMMockToolUtil.h"
#import "MMCgiMockCase.h"

@interface JSONEditorViewController ()
@property (nonatomic, weak) IBOutlet WKWebView *jsonEditorWebview;

@property(nonatomic, strong) MMCgiMockScript *mockScript;

@property(nonatomic, assign) BOOL isUpdateFromSvr;

@property (weak) IBOutlet NSButton *isUpdateFromSvrCheckBox;
@property (weak) IBOutlet NSButton *outputButton;

@end

@implementation JSONEditorViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
    self.title = @"JSON Editor";
}

- (void)setControllerType:(JSONEditorViewControllerType)controllerType {
    _controllerType = controllerType;
    if (_controllerType == JSONEditorViewControllerTypeEdit) {
        [self.outputButton setTitle:@"保存"];
    } else {
        [self.outputButton setTitle:@"输出脚本"];
    }
}

- (void)viewDidAppear {
    [super viewDidAppear];
    [self loadUrl];
}

- (void)setCgiMockScript:(MMCgiMockScript *)mockScript {
    self.mockScript = mockScript;
    if (self.mockScript.isUpdateFromSvr) {
        self.isUpdateFromSvr = YES;
        self.isUpdateFromSvrCheckBox.state = NSControlStateValueOn;
    } else {
        self.isUpdateFromSvr = NO;
        self.isUpdateFromSvrCheckBox.state = NSControlStateValueOff;
    }
}

- (void)loadUrl {
    NSString *jsonEditor = [[self jsonEditorPagePath] stringByAppendingPathComponent:@"/jsoneditor/examples/03_switch_mode.html"];
    
    //显示内容
    [self.jsonEditorWebview loadFileURL:[NSURL fileURLWithPath:jsonEditor] allowingReadAccessToURL:[NSURL fileURLWithPath:[NSString stringWithFormat:@"%@/jsoneditor/", [self jsonEditorPagePath]] isDirectory: YES]];
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.8 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [self delayLoadJSON];
    });
}

- (void)delayLoadJSON {
    NSString *jsScript = [NSString stringWithFormat:@"var json = %@;\neditor.set(json)", self.mockScript.jsonScript];
    [self.jsonEditorWebview evaluateJavaScript:jsScript completionHandler:nil];
}

- (NSString *)jsonEditorPagePath {
    NSString *rootDir = [MMMockToolUtil projectRootDir];
    NSString *targetPath = [rootDir stringByAppendingPathComponent:@"/tools/CgiMockTool/"];
    return targetPath;
}

- (IBAction)onClickOutputBtn:(id)sender {
    [self.jsonEditorWebview evaluateJavaScript:@"editor.get()" completionHandler:^(id _Nullable response, NSError * _Nullable error) {
        
        NSError *jsonError = nil;
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:(NSDictionary *)response options:NSJSONWritingPrettyPrinted error:&jsonError];
        if ([jsonData length] == 0 || error != nil) {
            NSLog(@"%@", jsonError);
        }
        NSString *jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
        jsonString = [@"var response = " stringByAppendingString:jsonString];
        
        jsonString = [jsonString stringByAppendingFormat:@"%@", [NSString stringWithFormat:@"\nmockRequest(%u).isUpdateFromSvr(%@).withResponse(response)", self.mockScript.cgiModel.cgiNumber, self.isUpdateFromSvr ? @"YES" : @"NO"]];
        
        self.mockScript.jsonScript = jsonString;
        [self saveGenerateMockScript];
    }];
}

- (void)saveGenerateMockScript {
    // create the save panel
    
    if (self.controllerType == JSONEditorViewControllerTypeEdit) {
        NSError *error = nil;
        //[self.mockScript]
        NSLog(@"%@", [NSString stringWithFormat:@"%@/MMTest/CgiMock/CgiMockTestCase/%@/%@", [MMMockToolUtil projectRootDir], self.mockScript.scriptDirectory, self.mockScript.scriptName]);
        [self.mockScript.jsonScript writeToFile:[NSString stringWithFormat:@"%@/MMTest/CgiMock/CgiMockTestCase/%@/%@", [MMMockToolUtil projectRootDir], self.mockScript.scriptDirectory, self.mockScript.scriptName] atomically:YES encoding:NSUTF8StringEncoding error:&error];
        if (error) {
            NSLog(@"%@", error);
        }
        [self dismissViewController:self];
        return;
    }
    
    NSSavePanel *panel = [NSSavePanel savePanel];
    
    // set a new file name
    if (self.mockScript.scriptName.length > 0) {
        [panel setNameFieldStringValue:self.mockScript.scriptName];
    } else {
        [panel setNameFieldStringValue:[NSString stringWithFormat:@"%@.js", self.mockScript.cgiModel.messageName]];
    }
    NSString *rootDir = [MMMockToolUtil projectRootDir];
    if (self.mockScript.scriptDirectory.length > 0) {
        [panel setDirectoryURL:[NSURL URLWithString:[NSString stringWithFormat:@"%@/MMTest/CgiMock/CgiMockTestCase/%@", rootDir, self.mockScript.scriptDirectory]]];
    } else {
        [panel setDirectoryURL:[NSURL URLWithString:[NSString stringWithFormat:@"%@/MMTest/CgiMock/CgiMockTestCase/", rootDir]]];
    }
    if ([panel runModal] == NSModalResponseOK) {

        NSError *error = nil;
        [self.mockScript.jsonScript writeToURL:[panel URL] atomically:YES encoding:NSUTF8StringEncoding error:&error];
        if (error != nil) {
            NSLog(@"error:%@", error.description);
        }
        [self dismissViewController:self];
    } else {
        NSLog(@"User click cancel.");
    }
}

- (IBAction)isUpdateFromSvr:(id)sender {
    NSButton *btn = (NSButton *)sender;
    self.isUpdateFromSvr = btn.state;
}

@end
