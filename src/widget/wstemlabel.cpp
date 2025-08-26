#include "wstemlabel.h"

#include "moc_wstemlabel.cpp"
#include "util/logger.h"

const mixxx::Logger kLogger("WStemLabel");

WStemLabel::WStemLabel(QWidget* pParent)
        : WLabel(pParent),
          m_stemInfo(QString(), QColor()),
          m_stemNo(0) {
}

void WStemLabel::setup(const QDomNode& node, const SkinContext& context) {
    m_stemNo = context.selectInt(node, "StemNum");

    VERIFY_OR_DEBUG_ASSERT(m_stemNo >= 1 && m_stemNo <= mixxx::kMaxSupportedStems) {
        SKIN_WARNING(node,
                context,
                QStringLiteral("StemNum is out of range. It should be between "
                               "1 and %1")
                        .arg(mixxx::kMaxSupportedStems));
        m_stemNo = qBound(1,
                m_stemNo,
                mixxx::kMaxSupportedStems); // Ensure m_stemNo is within the
                                            // valid range
    }
}

void WStemLabel::slotTrackUnloaded(TrackPointer) {
    m_stemInfo = StemInfo();
    updateLabel();
}

void WStemLabel::slotTrackLoaded(TrackPointer pTrack) {
    if (!pTrack) {
        return;
    }

    auto stemInfo = pTrack->getStemInfo();

    if (stemInfo.isEmpty()) {
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(m_stemNo <= stemInfo.size() + 1) {
        kLogger.warning() << "Stem number out of range. m_stemNo: " << m_stemNo
                          << ", stemInfo size: " << stemInfo.size();
        return;
    }

    //    qDebug() << "EVE -> m_stemInfo: " << m_stemInfo;
    if (m_stemNo == 1) {
        // QColor color = "#FFFFFF";
        QColor color(0xFFFFFF);
        QString text = "Pre-Mix";
        setTextColor(color);
        setLabelText(text);
    } else {
        // m_stemInfo = stemInfo[m_stemNo - 1];
        m_stemInfo = stemInfo[m_stemNo - 2];
        // qDebug() << "EVE -> m_stemNo: " << m_stemNo;
        // qDebug() << "EVE -> stemInfo [m_stemNo]: " << stemInfo[m_stemNo - 2];
        updateLabel();
    }
}

void WStemLabel::updateLabel() {
    QColor color = m_stemInfo.getColor();
    QString text = m_stemInfo.getLabel();
    setTextColor(color);
    setLabelText(text);
}

void WStemLabel::setTextColor(const QColor& color) {
    QPalette palette = this->palette();
    palette.setColor(QPalette::WindowText, color);
    this->setPalette(palette);
}

void WStemLabel::setLabelText(const QString& text) {
    this->setText(text);
}
