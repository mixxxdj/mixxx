#include "util/appleosversion.h"

#import <Foundation/Foundation.h>

QString getAppleOsVersion() {
    NSProcessInfo* processInfo = [NSProcessInfo processInfo];
    NSString* osVer = processInfo.operatingSystemVersionString;
    return QString::fromCFString((__bridge CFStringRef)osVer);
}
