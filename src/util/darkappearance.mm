#include "util/darkappearance.h"
#import <AppKit/NSApplication.h>

bool darkAppearance() {
    if (__builtin_available(macOS 10.14, *)) {
        auto appearance =
                [NSApp.effectiveAppearance bestMatchFromAppearancesWithNames:@[
                    NSAppearanceNameAqua,
                    NSAppearanceNameDarkAqua
                ]];
        return [appearance isEqualToString:NSAppearanceNameDarkAqua];
    }
    return false;
}
