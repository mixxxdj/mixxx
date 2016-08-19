#include "layout.h"
#include "utils.h"
#include <QDebug>

#include <X11/XKBlib.h>


Layout::Layout(const QString& varName, QString name, KeyboardLayoutPointer pData) :
        varName(varName),
        name(name) {

    qDebug() << "Loading layout " << name;

    // Copy layout data
    for (int i = 0; i < LAYOUT_LEN; i++) {
        data[i][0] = pData[i][0]; // Unmodified KbdKeyChar
        data[i][1] = pData[i][1]; // Shift modified KbdKeyChar
    }
}

Layout::Layout(const QString& varName, QString name) :
        varName(varName),
        name(name) {

    Display* display = XOpenDisplay(NULL);

    // Get keyboard data from X
    XkbDescPtr descPtr = XkbGetKeyboard(display, XkbAllComponentsMask, XkbUseCoreKbd);
    XkbClientMapPtr map = descPtr->map;
    KeySym* syms = map->syms;

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
            KeySym& keysym = syms[offset + i];

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

QStringList Layout::generateCode() const {
    QStringList lines;
    QString indent = "        ";

    lines.append(QString("// %1").arg(name));

    lines.append(
            QString("static const KbdKeyChar %1[%2][2] = {")
                    .arg(varName,QString::number(LAYOUT_LEN))
    );

    for (int i = 0; i < LAYOUT_LEN; i++) {
        int keycode = utils::layoutIndexToKeycode(i);
        QString keyName = utils::keycodeToKeyname(keycode);

        // If this key is the first key of the row, place an extra white
        // line and a comment telling which row we are talking about
        bool firstOfRow = keycode == TLDE || keycode == AD01 || keycode == AC01 || keycode == LSGT;
        if (firstOfRow) {
            QString rowName;
            if (keycode == TLDE)      rowName = "Digits row";
            else if (keycode == AD01) rowName = "Upper row";
            else if (keycode == AC01) rowName = "Home row";
            else if (keycode == LSGT) rowName = "Lower row";

            if (i > 0) lines.append("");
            lines.append(QString("%1// %2").arg(indent, rowName));
        }

        const KbdKeyChar& keyCharNoMods = data[i][0];
        const KbdKeyChar& keyCharShift = data[i][1];

        QString line = QString("%1/* %2 */ ").arg(indent, keyName);
        line += QString("{%1, %2}").arg(
                utils::createKbdKeyCharLiteral(keyCharNoMods),
                utils::createKbdKeyCharLiteral(keyCharShift)
        );

        // If not last, place a separation comma
        if (i < LAYOUT_LEN - 1) {
            line += ",";
        }

        lines.append(line);
    }

    lines.append("};");

    return lines;
}


Layout::~Layout() {}
