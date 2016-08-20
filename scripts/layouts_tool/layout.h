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

    QStringList generateCode() const;

    QString m_variableName;
    QString m_name;

  private:
    KbdKeyChar data[kLayoutLen][2];
};


#endif // LAYOUT_H
