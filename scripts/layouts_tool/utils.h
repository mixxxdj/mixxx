#ifndef UTILS_H
#define UTILS_H


#include <QApplication>
#include <QTextStream>
#include "layout.h"

typedef unsigned long KeySym;

namespace utils {
    void clearTerminal();
    QString keycodeToKeyname(int keycode);
    int keycodeToLayoutIndex(int keycode);
    int layoutIndexToKeycode(int layoutIndex);
    int keysymToUnicode(KeySym keysym);
    QString createKbdKeyCharLiteral(const KbdKeyChar &kbdKeyChar);
    QString inputLocaleName();

    inline QTextStream& qout() {
        static QTextStream out{stdout};
        return out;
    }

    inline QTextStream& qin() {
        static QTextStream in{stdin};
        return in;
    }
}

struct KeysymUnicodePair {
    unsigned int keysym;
    int unicodePosition;
};

#endif //LAYOUTS_TOOL_UTILS_H
