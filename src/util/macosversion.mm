#include "util/macosversion.h"

#import <Foundation/Foundation.h>

QString getMacOsVersion() {
    NSProcessInfo* processInfo = [NSProcessInfo processInfo];
    NSString* osVer = processInfo.operatingSystemVersionString;
    return QString::fromCFString((__bridge CFStringRef)osVer);
}
