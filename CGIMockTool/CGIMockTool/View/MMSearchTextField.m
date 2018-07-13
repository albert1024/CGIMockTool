//
//  MMSearchTextField.m
//  CGIMockTool
//
//  Created by 梁力图 on 2018/7/3.
//  Copyright © 2018 littleliang. All rights reserved.
//

#import "MMSearchTextField.h"

@implementation MMSearchTextField

- (BOOL)performKeyEquivalent:(NSEvent *)event {
    bool bHandled = NO;
    if (([event type] == NSEventTypeKeyDown) && ([event modifierFlags] & NSEventModifierFlagCommand)) {
        unsigned short keyCode = [event keyCode];
        //NSLog(@"%u", keyCode);
        switch (keyCode) {
            case 0: {
                if ([NSApp sendAction:@selector(selectAll:) to:nil from:self]) {
                    bHandled = YES;
                }
            }
                break;
            case 6: {
                if ([NSApp sendAction:@selector(undo:) to:nil from:self]) {
                    bHandled = YES;
                }
            }
                break;
            case 8 : {
                //NSRange range = [self selectedRange];
                if ([NSApp sendAction:@selector(copy:) to:nil from:self]) {
                    bHandled = YES;
                }
            }
                break;
            case 9 : {
                if ([NSApp sendAction:@selector(paste:) to:nil from:self]) {
                    bHandled = YES;
                }
            }
                break;
                
            default:
                break;
        }
    }
    
    return bHandled;
    
}

@end
