#ifndef LAYOUT_H
#define LAYOUT_H

#include <QString>

#include "defs.h"

struct KbdKeyChar {
    char16_t character;
    bool is_dead;
};

typedef const KbdKeyChar (*KeyboardLayoutPointer)[2];

class Layout {
public:
    Layout(QString varName, QString name, KeyboardLayoutPointer data);
    virtual ~Layout();

private:
    QString varName;
    QString name;
    KbdKeyChar data[LAYOUT_LEN][2];
};


#endif // LAYOUT_H
