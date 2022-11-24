#include "widget/wcoverartlabel.h"

#include <QtDebug>

#include "library/coverartutils.h"
#include "library/dlgcoverartfullsize.h"
#include "moc_wcoverartlabel.cpp"
#include "track/track.h"
#include "widget/wcoverartmenu.h"

namespace {

constexpr QSize kDeviceIndependentCoverLabelSize = QSize(100, 100);

inline QPixmap scaleCoverLabel(
        QWidget* parent,
        QPixmap pixmap) {
    const auto devicePixelRatioF = parent->devicePixelRatioF();
    pixmap.setDevicePixelRatio(devicePixelRatioF);
    return pixmap.scaled(
            kDeviceIndependentCoverLabelSize * devicePixelRatioF,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
}

QPixmap createDefaultCover(QWidget* parent) {
    auto defaultCover = QPixmap(CoverArtUtils::defaultCoverLocation());
    return scaleCoverLabel(parent, defaultCover);
}

} // anonymous namespace

WCoverArtLabel::WCoverArtLabel(QWidget* parent, WCoverArtMenu* pCoverMenu)
        : QLabel(parent),
          m_pCoverMenu(pCoverMenu),
          m_pDlgFullSize(make_parented<DlgCoverArtFullSize>(this, nullptr, pCoverMenu)),
          m_defaultCover(createDefaultCover(this)) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setFrameShape(QFrame::Box);
    setAlignment(Qt::AlignCenter);
    setPixmap(m_defaultCover);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,
            &WCoverArtLabel::customContextMenuRequested,
            this,
            &WCoverArtLabel::slotCoverMenu);
}

WCoverArtLabel::~WCoverArtLabel() = default;

void WCoverArtLabel::setCoverArt(const CoverInfo& coverInfo,
        const QPixmap& px) {
    if (m_pCoverMenu) {
        m_pCoverMenu->setCoverArt(coverInfo);
    }
    if (px.isNull()) {
        m_loadedCover = px;
        setPixmap(m_defaultCover);
    } else {
        m_loadedCover = scaleCoverLabel(this, px);
        setPixmap(m_loadedCover);
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    QSize frameSize = pixmap(Qt::ReturnByValue).size() / devicePixelRatioF();
#else
    QSize frameSize = pixmap()->size() / devicePixelRatioF();
#endif
    frameSize += QSize(2, 2); // margin
    setMinimumSize(frameSize);
    setMaximumSize(frameSize);

}

void WCoverArtLabel::slotCoverMenu(const QPoint& pos) {
    if (m_pCoverMenu) {
        m_pCoverMenu->popup(mapToGlobal(pos));
    }
}

void WCoverArtLabel::loadTrack(TrackPointer pTrack) {
    m_pLoadedTrack = pTrack;
}

void WCoverArtLabel::loadData(const QByteArray& data) {
    m_Data = data;
}

void WCoverArtLabel::mousePressEvent(QMouseEvent* event) {
    if (m_pCoverMenu && m_pCoverMenu->isVisible()) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (m_pDlgFullSize->isVisible()) {
            m_pDlgFullSize->close();
        } else {
            if (m_loadedCover.isNull()) {
                return;
            } else if (!m_pLoadedTrack && !m_Data.isNull()) {
                m_pDlgFullSize->showFetchedCoverArt(m_Data);
            } else {
                m_pDlgFullSize->showTrackCoverArt(m_pLoadedTrack);
            }
        }
    }
}
