#include "util/screensaverios.h"

#import <UIKit/UIKit.h>

namespace mixxx {

void setIdleTimerDisabled(bool disabled) {
    [[UIApplication sharedApplication] setIdleTimerDisabled:disabled];
}

}; // namespace mixxx
