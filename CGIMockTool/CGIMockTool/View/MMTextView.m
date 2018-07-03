//
//  MMTextView.m
//  CGIMockTool
//
//  Created by littleliang on 2018/6/14.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import "MMTextView.h"

@implementation MMTextView

- (BOOL)performKeyEquivalent:(NSEvent *)event {
    bool bHandled = NO;
    if (([event type] == NSEventTypeKeyDown) && ([event modifierFlags] & NSEventModifierFlagCommand)) {
        unsigned short keyCode = [event keyCode];
        //NSLog(@"%u", keyCode);
        switch (keyCode) {
            case 0: {
                [self selectAll:nil];
                bHandled = YES;
            }
                break;
            case 6: {
                if ([[self undoManager] canUndo]) {
                    [[self undoManager] undo];
                    bHandled = YES;
                }
            }
                break;
            case 8 : {
                //NSRange range = [self selectedRange];
                [self copy:self];
                bHandled = YES;
            }
                break;
            case 9 : {
                [self paste:self];
                bHandled = YES;
            }
                break;
            
            default:
                break;
        }
    }
    
    return bHandled;
          
}
@end
