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

    // If not null, Mixxx will map on the position of this key sequence's key.
    // Hence the scancode of the key. In order for Mixxx to find the key's
    // scancode it needs to know which keyboard layout this key sequence is
    // targeted on. That layout will be stored in byPositionOf, found in the
    // 'byPositionOf' attribute of the <keyseq> element
    QString byPositionOf;
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
    QList<int> findScancodesForCharacter(KeyboardLayoutPointer pLayout,
                                         const QChar &character,
                                         Qt::KeyboardModifier modifier);
    QString keyseqGetKey(const QString &keysequence);
    QStringList keyseqGetModifiers(const QString &keyseq);
}


#endif // MIXXX_LAYOUTUTILS_H
