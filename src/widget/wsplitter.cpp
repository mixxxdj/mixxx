#include "widget/wsplitter.h"

WSplitter::WSplitter(QWidget* pParent)
        : QSplitter(pParent),
          WBaseWidget(this) {
}

WSplitter::~WSplitter() {
}

void WSplitter::setup(QDomNode node, const SkinContext& context) {
    // Default orientation is horizontal.
    if (context.hasNode(node, "Orientation")) {
        QString layout = context.selectString(node, "Orientation");
        if (layout == "vertical") {
            setOrientation(Qt::Vertical);
        } else if (layout == "horizontal") {
            setOrientation(Qt::Horizontal);
        }
    }
}

bool WSplitter::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QWidget::event(pEvent);
}
