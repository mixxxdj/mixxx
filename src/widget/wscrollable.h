#pragma once

#include <QDomNode>
#include <QEvent>
#include <QScrollArea>

#include "preferences/usersettings.h"
#include "skin/legacy/skincontext.h"
#include "widget/wbasewidget.h"

/// Creates a QScrollArea. The QT default is to show scrollbars if needed.
class WScrollable : public QScrollArea, public WBaseWidget {
    Q_OBJECT
  public:
    WScrollable(QWidget* pParent);

    /// WScrollable supports the following attributes: HorizontalScrollBarPolicy, VerticalScrollBarPolicy
    void setup(const QDomNode& node, const SkinContext& context);
};
