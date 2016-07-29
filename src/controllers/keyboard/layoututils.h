#ifndef MIXXX_LAYOUTUTILS_H
#define MIXXX_LAYOUTUTILS_H

#include <QString>

namespace layoutUtils {
    typedef const KbdKeyChar (*KeyboardLayoutPointer)[2];

    unsigned char layoutIndexToScancode(const int layoutIndex);
    unsigned char scancodeToLayoutIndex(unsigned char scancode);
    KeyboardLayoutPointer getLayout(const QString layoutName);

    struct KbdKeyChar {
        char16_t character;
        bool is_dead;
    };
}


#endif // MIXXX_LAYOUTUTILS_H
