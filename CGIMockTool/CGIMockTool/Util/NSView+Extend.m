//
//  NSView+Extend.m
//  CGIMockTool
//
//  Created by littleliang on 2018/6/11.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import "NSView+Extend.h"

@implementation NSView (Extend)

- (void)removeAllSubviews {
    for (NSView *subview in [self.subviews copy]) {
        [subview removeFromSuperview];
    }
}


- (void)hideAllSubviews {
    for (NSView *subview in [self.subviews copy]) {
        subview.hidden = YES;
    }
}

- (void)showAllSubviews {
    for (NSView *subview in [self.subviews copy]) {
        subview.hidden = NO;
    }
}


@end
