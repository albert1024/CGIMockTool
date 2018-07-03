//
//  main.m
//  CGIMockTool
//
//  Created by littleliang on 2018/6/9.
//  Copyright © 2018年 littleliang. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef _MSC_VER
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <errno.h>
#include <iostream>
#include <ctype.h>

#define O_BINARY 0

int main(int argc, const char * argv[]) {
    return NSApplicationMain(argc, argv);
}
