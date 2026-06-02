#include "wstemcontrolborder.h"

#include <QColor>
#include <QLayout>
#include <QString>

#include "moc_wstemcontrolborder.cpp"
#include "util/logger.h"

WStemControlBorder::WStemControlBorder(QWidget* pParent)
        : WWidgetGroup(pParent) {
    setContentsMargins(0, 0, 0, 0);
}

void WStemControlBorder::setup(const QDomNode& node, const SkinContext& context) {
    WWidgetGroup::setup(node, context);

    m_stemNo = context.selectInt(node, "StemNum");
    m_group = context.selectString(node, "Group");

    if (m_stemNo < 1 || m_stemNo > mixxx::kMaxSupportedStems) {
        m_stemNo = qBound(1, m_stemNo, static_cast<int>(mixxx::kMaxSupportedStems));
    }

    if (objectName().isEmpty()) {
        setObjectName(QString("StemControlBorder_%1_%2").arg(m_group).arg(m_stemNo));
    }

    QLayout* pLayout = layout();
    if (pLayout) {
        pLayout->setContentsMargins(4, 2, 4, 2);
        pLayout->setSpacing(4);
    } else {
        auto* pNewLayout = new QHBoxLayout(this);
        pNewLayout->setContentsMargins(4, 2, 4, 2);
        pNewLayout->setSpacing(4);
        setLayout(pNewLayout);
    }

    updateBorderStyle(QColor(0x888888));

    qDebug() << "WStemControlBorder:" << objectName() << "has" << children().size() << "children";
}

void WStemControlBorder::slotTrackLoaded(TrackPointer pTrack) {
    if (!pTrack) {
        slotTrackUnloaded();
        return;
    }

    auto stemInfo = pTrack->getStemInfo();
    if (stemInfo.isEmpty()) {
        slotTrackUnloaded();
        return;
    }

    if (m_stemNo > stemInfo.size() + 1) {
        slotTrackUnloaded();
        return;
    }

    int index = m_stemNo - 1;
    if (index >= 0 && index < stemInfo.size()) {
        m_stemInfo = stemInfo[index];
        updateBorderStyle(m_stemInfo.getColor());
    }
}

void WStemControlBorder::slotTrackUnloaded() {
    m_stemInfo = StemInfo();
    updateBorderStyle(QColor(0x888888));
}

void WStemControlBorder::updateBorderStyle(const QColor& color) {
    if (!color.isValid()) {
        setStyleSheet(
                QString("WStemControlBorder#%1 { border: 2px solid #888888; "
                        "border-radius: 4px; background: rgba(0,0,0,0.1); }")
                        .arg(objectName()));
        return;
    }

    QString borderStyle =
            QString("WStemControlBorder#%1 { border: 2px solid %2; "
                    "border-radius: 4px; background: rgba(0,0,0,0.1); }")
                    .arg(objectName(), color.name());

    // full rectangle
    // QString style = QString("WStemControlBorder#%1 { background-color: %2; }")
    //                        .arg(this->objectName(), color.name());
    setStyleSheet(borderStyle);

    // qDebug() << "[WStemControlBorder] -> Border color for" << objectName() <<
    // "set to" << color.name();
}
