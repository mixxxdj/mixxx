#include <QList>

#include "widget/wsplitter.h"

WSplitter::WSplitter(QWidget* pParent, ConfigObject<ConfigValue> *pConfig)
        : QSplitter(pParent),
          m_pConfig(pConfig),
          WBaseWidget(this) {
    connect(this, SIGNAL(splitterMoved(int,int)),
            this, SLOT(slotSplitterMoved()));
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
    return QSplitter::event(pEvent);
}

void WSplitter::slotSplitterMoved() {
    QList<int> width = sizes();
    m_pConfig->set(ConfigKey("[Skin]", "SplitSizes"),
                   ConfigValue(QString("%1, %2")
                               .arg(width.at(0))
                               .arg(width.at(1))));
}
