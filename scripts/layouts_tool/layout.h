#ifndef LAYOUT_H
#define LAYOUT_H

#include <QString>
#include <QX11Info>
#include <QStringList>
#include "defs.h"

struct KbdKeyChar {
    char16_t character;
    bool isDead;
};

typedef const KbdKeyChar (*KeyboardLayoutPointer)[2];

class Layout {
  public:
    Layout(const QString& variableName, QString name, KeyboardLayoutPointer pData);
    Layout(const QString& variableName, QString name);
    virtual ~Layout();

    // This name represents the variable name
    // of the layout in the layouts cpp file
    QString m_variableName;

    // This name represents the name as described in the comment
    // just above the layout declaration in the layouts cpp file
    QString m_name;

    KbdKeyChar m_data[kLayoutLen][2];
};


#endif // LAYOUT_H
