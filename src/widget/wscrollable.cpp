#include "widget/wscrollable.h"

WScrollable::WScrollable(QWidget* pParent)
        : QScrollArea(pParent),
          WBaseWidget(this) {
}

void WScrollable::setup(const QDomNode& node, const SkinContext& context) {
    QString horizontalPolicy;
    // The QT default is "As Needed", so we don't need a selector for that.
    if (context.hasNodeSelectString(node, "HorizontalScrollBarPolicy", &horizontalPolicy)) {
        if (horizontalPolicy == "on") {
            setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        } else if (horizontalPolicy == "off") {
            setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }
    QString verticalPolicy;
    if (context.hasNodeSelectString(node, "VerticalScrollBarPolicy", &verticalPolicy)) {
        if (verticalPolicy == "on") {
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        } else if (verticalPolicy == "off") {
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }
}
