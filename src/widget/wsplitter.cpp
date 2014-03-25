#include <QList>

#include "widget/wsplitter.h"

WSplitter::WSplitter(QWidget* pParent)
        : QSplitter(pParent),
          WBaseWidget(this) {
}

WSplitter::~WSplitter() {
}

void WSplitter::setup(QDomNode node,
                      const SkinContext& context,
                      ConfigObject<ConfigValue> *pConfig) {
    m_pConfig = pConfig;
    // it only connects after initialize m_pConfig
    connect(this, SIGNAL(splitterMoved(int,int)),
            this, SLOT(slotSplitterMoved()));

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
    m_pConfig->set(ConfigKey("[Skin]","SplitSizes"),
                   ConfigValue(QString("%1,%2")
                               .arg(width.at(0))
                               .arg(width.at(1))));
}
