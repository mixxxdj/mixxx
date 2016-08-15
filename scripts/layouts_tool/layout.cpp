#include "layout.h"
#include "utils.h"
#include <QDebug>

#include <X11/XKBlib.h>


Layout::Layout(QString varName, QString name, KeyboardLayoutPointer pData) :
        varName(varName),
        name(name) {

    qDebug() << "Loading layout " << name;

    // Copy layout data
    for (int i = 0; i < LAYOUT_LEN; i++, pData++) {
        data[i][0] = pData[i][0]; // Unmodified KbdKeyChar
        data[i][1] = pData[i][1]; // Shift modified KbdKeyChar
    }
}

Layout::Layout(QString varName, QString name) :
        varName(varName),
        name(name) {

    Display *display = XOpenDisplay(NULL);

    // Get keyboard data from X
    XkbDescPtr descPtr = XkbGetKeyboard(display, XkbAllComponentsMask, XkbUseCoreKbd);
    XkbClientMapPtr map = descPtr->map;
    KeySym *syms = map->syms;

    for (int keycode = AE01; keycode <= LSGT; keycode++) {

        // Skip Backspace and Enter
        if (utils::keycodeToKeyname(keycode).isEmpty()) {
            continue;
        }

        // Get keysyms for current key
        _XkbSymMapRec& keysymMap = map->key_sym_map[keycode];

        // Get keysym offset (the index of this keysym in syms)
        unsigned short offset = keysymMap.offset;
        if (!offset) continue;

        // Retrieve layout index for key code
        int layoutIndex = utils::keycodeToLayoutIndex(keycode);
        if (layoutIndex == -1) {
            qDebug() << QString("Layout index for key code %1 not found").arg(keycode);
            continue;
        }

        // Iterate over keysyms of current key
        for (int i = 0; i < keysymMap.width; i++) {

            // We are only interested in keysym without any
            // modifier and keysym for shift modifier.
            if (i > 1) {
                break;
            }

            // Get keysym
            KeySym &keysym = syms[offset + i];

            // Get name of keysym
            std::string keysymName = XKeysymToString(keysym);

            // Check whether this is a dead key or not
            bool isDeadKey = keysymName.compare(0, 4, "dead") == 0;

            // Construct final package, ready to be added to the layout
            KbdKeyChar kbdKeyChar;
            kbdKeyChar.character = (char16_t) keysym;
            kbdKeyChar.is_dead = isDeadKey;

            // Load KbdKeyChar
            data[layoutIndex][i] = kbdKeyChar;
        }
    }

}


Layout::~Layout() {}
