#ifndef MIXXX_LAYOUTUTILS_H
#define MIXXX_LAYOUTUTILS_H

#include <QString>
#include "control/controlobject.h"
#include "controllers/keyboard/layouts.h"

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

typedef unsigned char Scancode;

namespace layoutUtils {
    // Get given layout index' scancode. Layout indexes greater than or equal
    // to kLayoutLen result in undefined behaviour.
    Scancode layoutIndexToScancode(const unsigned char layoutIndex);

    // Get given scancode's layout index. Return -1 if layout index was not found.
    int scancodeToLayoutIndex(const Scancode scancode);

    // Get KbdKeyChar for key with given scancode and modifier on given layout. Note that
    // only Qt::NoModifier and Qt::ShiftModifier are supported, because our layout tables
    // do only contain character information for those two modifiers.
    const KbdKeyChar* getKbdKeyChar(KeyboardLayoutPointer pLayout,
                                    Scancode scancode,
                                    Qt::KeyboardModifier modifier);

    // Find scancodes of keys that match given character and modifier on given layout.
    // This will be most of the times just one scancode, since most keyboards don't have
    // multiple keys sharing the same keysym or character. Not that also this function
    // accepts only Qt::NoModifier and Qt::ShiftModifier for same reasons as getKeyChar.
    QList<Scancode> findScancodesForCharacter(KeyboardLayoutPointer pLayout,
                                              const QChar &character,
                                              Qt::KeyboardModifier modifier);

    // Get key of given key sequence
    QString keyseqGetKey(const QString& keyseq);

    // Get modifiers of given key sequence
    QStringList keyseqGetModifiers(const QString& keyseq);
}


#endif // MIXXX_LAYOUTUTILS_H
