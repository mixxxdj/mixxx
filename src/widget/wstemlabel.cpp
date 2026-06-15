#include "wstemlabel.h"

#include "library/dlgtagfetcher.h"
#include "library/dlgtrackinfo.h"
#include "moc_wstemlabel.cpp"
#include "track/track.h"
#include "util/assert.h"
#include "util/logger.h"
#include "util/parented_ptr.h"

const mixxx::Logger kLogger("WStemLabel");

WStemLabel::WStemLabel(QWidget* pParent, UserSettingsPointer pConfig)
        : WLabel(pParent),
          m_stem(QString(), QColor()),
          m_pConfig(pConfig),
          m_stemNo(0) {
}

void WStemLabel::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
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

void WStemLabel::slotTrackUnloaded(TrackPointer oldTrack) {
    DEBUG_ASSERT(oldTrack == m_pTrack);
    m_pTrack.reset();
    m_stem = mixxx::Stem();
    updateLabel();
    oldTrack->disconnect(nullptr, this);
}

void WStemLabel::slotTrackLoaded(TrackPointer pTrack) {
    m_pTrack = pTrack;
    if (!pTrack) {
        return;
    }

    auto stemInfo = pTrack->getStemInfo();

    if (!stemInfo.isValid()) {
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(m_stemNo <= stemInfo.size()) {
        kLogger.warning() << "Stem number out of range. m_stemNo: " << m_stemNo
                          << ", stemInfo size: " << stemInfo.size();
        return;
    }

    m_stem = stemInfo[m_stemNo - 1];
    updateLabel();

    connect(pTrack.get(), &Track::stemInfoChanged, this, [this, pTrack]() {
        m_stem = pTrack->getStemInfo()[m_stemNo - 1];
        updateLabel();
    });
}

void WStemLabel::mouseDoubleClickEvent(QMouseEvent*) {
    if (!m_stem.isValid()) {
        return;
    }
    // Use the single-track editor with Next/Prev buttons and DlgTagFetcher.
    // Create a fresh dialog on invocation.
    m_pDlgTrackInfo = std::make_unique<DlgTrackInfo>(
            m_pConfig);
    connect(m_pDlgTrackInfo.get(),
            &QDialog::finished,
            this,
            [this]() {
                if (m_pDlgTrackInfo.get() == sender()) {
                    m_pDlgTrackInfo.release()->deleteLater();
                }
            });
    m_pDlgTrackInfo->loadTrack(m_pTrack);
    m_pDlgTrackInfo->show();
    m_pDlgTrackInfo->focusField(QStringLiteral("stem_%0").arg(m_stemNo - 1));
}

void WStemLabel::updateLabel() {
    QColor color = m_stem.getColor();
    QString text = m_stem.getLabel();
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
