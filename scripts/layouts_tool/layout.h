#ifndef LAYOUT_H
#define LAYOUT_H

#include <QString>
#include <QX11Info>
#include <QStringList>
#include "defs.h"

struct KbdKeyChar {
    char16_t character;
    bool is_dead;
};

typedef const KbdKeyChar (*KeyboardLayoutPointer)[2];

class Layout {
  public:
    Layout(const QString& varName, QString name, KeyboardLayoutPointer pData);
    Layout(const QString& varName, QString name);
    virtual ~Layout();

    QStringList generateCode() const;

    QString varName;
    QString name;

  private:
    KbdKeyChar data[kLayoutLen][2];
};


#endif // LAYOUT_H
