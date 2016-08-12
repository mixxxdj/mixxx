#ifndef LAYOUT_H
#define LAYOUT_H

#include <QString>

#include "defs.h"

struct KbdKeyChar {
    char16_t character;
    bool is_dead;
};

class Layout {
public:
    Layout();
    virtual ~Layout();

private:
    const QString name;
    const QString varName;
    KbdKeyChar data[LAYOUT_LEN][2];
};

typedef const KbdKeyChar (*KeyboardLayoutPointer)[2];

#endif // LAYOUT_H
