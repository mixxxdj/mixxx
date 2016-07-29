#include "layoututils.h"

namespace layoutUtils {
    static const unsigned char LAYOUT_SCANCODES[48] = {
            0x29, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,  // Digits row
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b,        // Upper row
            0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x2b,        // Home row
            0x5e, 0x0c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35               // Lower row

            // Note: Although the <LSGT> key is not defined in the scancode set 2,
            // it is added for pc105 compatibility. It's given scancode 0x5e (94)
    };

    unsigned char layoutIndexToScancode(unsigned char layoutIndex) {
        return LAYOUT_SCANCODES[layoutIndex];
    };

    unsigned char scancodeToLayoutIndex(unsigned char scancode) {
        for (unsigned char i = 0; i < 48; i++) {
            if (LAYOUT_SCANCODES[i] == scancode) {
                return i;
            }
        }
        return (unsigned char) -1;
    }
}