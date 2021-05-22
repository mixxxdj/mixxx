#pragma once

#include <QDomNode>
#include <QEvent>
#include <QScrollArea>

#include "preferences/usersettings.h"
#include "skin/skincontext.h"
#include "widget/wbasewidget.h"

class WScrollable : public QScrollArea, public WBaseWidget {
    Q_OBJECT
  public:
    WScrollable(QWidget* pParent);
};
