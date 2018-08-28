#import "MZipUtil.h"
#include "zip.h"
#include "unzip.h"
#import "zlib.h"
#import "zconf.h"
#import "ZipArchive.h"

@implementation MZipUtil

+ (BOOL)UnZipFile:(NSString *)path toPath:(NSString *)destination {
	return [self UnZipFile:path toPath:destination overwrite:YES password:nil error:nil];
}


+ (BOOL)UnZipFile:(NSString *)path toPath:(NSString *)destination overwrite:(BOOL)overwrite password:(NSString *)password error:(NSError **)error
{
    ZipArchive *zip = [[ZipArchive alloc] init];

    if (![zip UnzipOpenFile:path Password:password]) {
        NSLog(@"UnZipFile, CreateZipFile2 fail");
        return NO;
    }
    
    if (![zip UnzipFileTo:destination overWrite:overwrite]) {
        NSLog(@"UnZipFile, UnzipFileTo fail");
        [zip UnzipCloseFile];
        return NO;
    }
    
    [zip UnzipCloseFile];
    return YES;
}

//+ (BOOL)ZipFile:(NSString *)path toPath:(NSString *)destination {
//    ZipArchive *zip = [[ZipArchive alloc] init];
//    
//    [zip CreateZipFile2:path];
//    [zip addFileToZip:<#(NSString *)#> newname:<#(NSString *)#>]
//}

+ (NSDate *)dateWithDosDate:(NSUInteger)value
{
    NSCalendar* gregorian = [[NSCalendar alloc] initWithCalendarIdentifier:NSCalendarIdentifierGregorian];
    NSDateComponents* dc = [[NSDateComponents alloc] init];
    NSUInteger uDate = (NSUInteger)(value >> 16);
    
    dc.day = (NSUInteger)(uDate & 0x1f);
    dc.month =  (NSUInteger)(((uDate) & 0x1E0) / 0x20);
    dc.year = (NSUInteger)(((uDate&0x0FE00) / 0x0200) + 1980);
    dc.hour = (NSUInteger) ((value & 0xF800) / 0x800);
    dc.minute =  (NSUInteger) ((value & 0x7E0) / 0x20);
    dc.second =  (NSUInteger) (2 * (value & 0x1f));
    
    return [gregorian dateFromComponents:dc];
}

@end
