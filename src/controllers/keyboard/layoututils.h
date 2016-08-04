#ifndef MIXXX_LAYOUTUTILS_H
#define MIXXX_LAYOUTUTILS_H

#include <QString>
#include "control/controlobject.h"

struct KbdKeyChar {
    char16_t character;
    bool is_dead;
};

// Representation of one <keyseq> element, parented to <control> elements
struct KbdControllerPresetKeyseq {
    // String representation of the key sequence (for example: Ctrl+a),
    // found in the text of the <keyseq> element
    QString keysequence;

    // Keyboard layout at which this key sequence is targetted (for example: es_ES),
    // found in the lang attribute of the <keyseq> element
    QString lang;

    // Scancode following the scancode-set 2:
    // https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html#ss1.4
    // The scancode is extracted from the scancode attribute of the <keyseq> element
    QString scancode;
};

// Representaion of one <control> element
struct KbdControllerPresetControl {
    // Combination of groupname as defined in the 'name' attribute
    // of the parent <group> element and the key as defined in the 'action'
    // attribute of the <control> element
    ConfigKey configKey;

    // <keyseq> childs, normally containing just one. If the key sequence is
    // explicitly overloaded for a specific keyboard layout, there will be an
    // extra one (or more)
    QList<KbdControllerPresetKeyseq> keyseqs;
};

typedef const KbdKeyChar (*KeyboardLayoutPointer)[2];

namespace layoutUtils {
    unsigned char layoutIndexToScancode(const unsigned char layoutIndex);
    unsigned char scancodeToLayoutIndex(const unsigned char scancode);
    KeyboardLayoutPointer getLayout(const QString layoutName);
    const KbdKeyChar* getKbdKeyChar(KeyboardLayoutPointer pLayout,
                                    unsigned char scancode,
                                    Qt::KeyboardModifier modifier);
    QString getCharFromKeysequence(QString keysequence);
    QStringList getModifiersFromKeysequence(QString keyseq);
}


#endif // MIXXX_LAYOUTUTILS_H
