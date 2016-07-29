#ifndef MIXXX_LAYOUTUTILS_H
#define MIXXX_LAYOUTUTILS_H

#include <QString>

struct KbdKeyChar {
    char16_t character;
    bool is_dead;
};

typedef const KbdKeyChar (*KeyboardLayoutPointer)[2];

namespace layoutUtils {
    unsigned char layoutIndexToScancode(const unsigned char layoutIndex);
    unsigned char scancodeToLayoutIndex(const unsigned char scancode);
    KeyboardLayoutPointer getLayout(const QString layoutName);
    KbdKeyChar getKbdKeyChar(KeyboardLayoutPointer pLayout,
                             unsigned char scancode,
                             Qt::KeyboardModifier modifier);
}


#endif // MIXXX_LAYOUTUTILS_H
