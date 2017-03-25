#ifndef WMENUBAR_H
#define WMENUBAR_H

#include <QWidget>
#include <QMenuBar>
#include <QList>
#include <QMenu>

#include "widget/wbasewidget.h"

class WMenuBar : public QMenuBar, public WBaseWidget
{
    Q_OBJECT
public:
    WMenuBar(QWidget* pParent, QList<QMenu*> pMenus);
};

#endif // WMENUBAR_H
