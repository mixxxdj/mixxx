#ifndef MIXXX_LAYOUTUTILS_H
#define MIXXX_LAYOUTUTILS_H

#include <QString>
#include "control/controlobject.h"
#include "controllers/keyboard/layouts.h"

struct KbdKeyChar {
    char16_t character;
    bool isDead;
};

// Representation of one <keyseq> element, parented to <control> elements
struct KbdControllerPresetKeyseq {
    // String representation of the key sequence (for example: Ctrl+a),
    // found in the text of the <keyseq> element
    QString keysequence;

    // Keyboard layout at which this key sequence is targeted (for example: es_ES),
    // found in the 'lang' attribute of the <keyseq> element
    QString lang;

    // Scancode following the scancode-set 2:
    // https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html#ss1.4
    // The scancode is extracted from the 'scancode' attribute of the <keyseq> element
    QString scancode;

    // Indicates whether this key sequence should be translated or not. For example,
    // key sequences like 'Ctrl+F1', 'Left', 'Shift+Right' or 'Space' do not need
    // translation. In those cases, final will be true. It is extracted from the
    // 'final' attribute of the <keyseq> element, where '1' => true and everything
    // else => false (if no final attribute => false).
    //
    // Note: Final key sequences usually don't specify neither lang nor scancode.
    bool final;
};

// Representation of one <control> element
struct KbdControllerPresetControl {
    // Combination of group name as defined in the 'name' attribute
    // of the parent <group> element and the key as defined in the 'action'
    // attribute of the <control> element
    ConfigKey configKey;

    // <keyseq> children, normally containing just one. If the key sequence is
    // explicitly overloaded for a specific keyboard layout, there will be an
    // extra one (or more)
    QList<KbdControllerPresetKeyseq> keyseqs;
};

namespace layoutUtils {
    unsigned char layoutIndexToScancode(const unsigned char layoutIndex);
    unsigned char scancodeToLayoutIndex(const unsigned char scancode);
    const KbdKeyChar* getKbdKeyChar(KeyboardLayoutPointer pLayout,
                                    unsigned char scancode,
                                    Qt::KeyboardModifier modifier);
    QString getCharFromKeysequence(const QString& keysequence);
    QStringList getModifiersFromKeysequence(const QString& keyseq);
}


#endif // MIXXX_LAYOUTUTILS_H
