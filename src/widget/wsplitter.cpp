#include <QList>

#include "widget/wsplitter.h"

WSplitter::WSplitter(QWidget* pParent, ConfigObject<ConfigValue> *pConfig)
        : QSplitter(pParent),
          WBaseWidget(this),
          m_pConfig(pConfig) {
    connect(this, SIGNAL(splitterMoved(int,int)),
            this, SLOT(slotSplitterMoved()));
}

WSplitter::~WSplitter() {
}

void WSplitter::setup(QDomNode node, const SkinContext& context) {
    // Load split sizes
    QString sizesJoined;
    QString msg;
    bool ok = false;
    // Try to load last values stored in mixxx.cfg
    if (context.hasNode(node, "SplitSizesConfigKey")) {
        m_configKey = ConfigKey::parseCommaSeparated(
                    context.selectString(node, "SplitSizesConfigKey"));

        if (m_pConfig->exists(m_configKey)) {
            sizesJoined = m_pConfig->getValueString(m_configKey);
            msg = "Reading .cfg file: '"
                    + m_configKey.group + " "
                    + m_configKey.item + " "
                    + sizesJoined
                    + "' does not match the number of children nodes:"
                    + QString::number(this->count());
            ok = true;
        }
    }
    // nothing in mixxx.cfg? Load default values
    if (!ok && context.hasNode(node, "SplitSizes")) {
        sizesJoined = context.selectString(node, "SplitSizes");
        msg = "<SplitSizes> for <Splitter> ("
                + sizesJoined
                + ") does not match the number of children nodes:"
                + QString::number(this->count());
    }
    // found some value for splitsizes?
    if (sizesJoined != NULL) {
        QStringList sizesSplit = sizesJoined.split(",");
        QList<int> sizesList;
        ok = false;
        foreach (const QString& sizeStr, sizesSplit) {
            sizesList.push_back(sizeStr.toInt(&ok));
            if (!ok) {
                break;
            }
        }
        if (sizesList.length() != this->count()) {
            SKIN_WARNING(node, context) << msg;
            ok = false;
        }
        if (ok) {
            this->setSizes(sizesList);
        }
    }

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

void WSplitter::slotSplitterMoved() {
    if (!m_configKey.group.isEmpty() && !m_configKey.item.isEmpty()) {
        QStringList sizeStrList;
        foreach (const int& sizeInt, sizes()) {
            sizeStrList.push_back(QString::number(sizeInt));
        }
        QString sizesStr = sizeStrList.join(",");
        m_pConfig->set(m_configKey, ConfigValue(sizesStr));
    }
}

bool WSplitter::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QSplitter::event(pEvent);
}
