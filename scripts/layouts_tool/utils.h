#ifndef UTILS_H
#define UTILS_H


#include <QApplication>

typedef unsigned long KeySym;

namespace utils {
    void clearTerminal();
    QString keycodeToKeyname(int keycode);
    int keycodeToLayoutIndex(int keycode);
    int keysymToUnicode(KeySym keysym);
    QString inputLocaleName();
}

struct KeysymUnicodePair {
    unsigned int keysym;
    int unicodePosition;
};

#endif //LAYOUTS_TOOL_UTILS_H
