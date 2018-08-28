#import <AppKit/AppKit.h>


@interface MZipUtil : NSObject

+ (BOOL)UnZipFile:(NSString *)path toPath:(NSString *)destination;

+ (BOOL)UnZipFile:(NSString *)path toPath:(NSString *)destination overwrite:(BOOL)overwrite password:(NSString *)password error:(NSError **)error;

+ (BOOL)ZipFile:(NSString *)path toPath:(NSString *)destination;

@end
