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

WCoverArtLabel::WCoverArtLabel(
        QWidget* parent, WCoverArtMenu* pCoverMenu, UserSettingsPointer pConfig)
        : QLabel(parent),
          m_pCoverMenu(new WCoverArtMenu(this, pConfig)),
          m_pDlgFullSize(make_parented<DlgCoverArtFullSize>(
                  this, nullptr, pCoverMenu, pConfig)),
          m_defaultCover(createDefaultCover(this)) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setFrameShape(QFrame::Box);
    setAlignment(Qt::AlignCenter);
    setPixmap(m_defaultCover);
}

WCoverArtLabel::~WCoverArtLabel() = default;

void WCoverArtLabel::setCoverArt(const CoverInfo& coverInfo,
        const QPixmap& px) {
    if (m_pCoverMenu != nullptr) {
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
    if (m_pCoverMenu == nullptr) {
        return;
    }
    m_pCoverMenu->popup(mapToGlobal(pos));
}

void WCoverArtLabel::contextMenuEvent(QContextMenuEvent* event) {
    if (m_pCoverMenu == nullptr) {
        return;
    }
    event->accept();
    m_pCoverMenu->popup(event->globalPos());
}

void WCoverArtLabel::loadTrack(TrackPointer pTrack) {
    m_pLoadedTrack = pTrack;
}

void WCoverArtLabel::mousePressEvent(QMouseEvent* event) {
    if (m_pCoverMenu != nullptr && m_pCoverMenu->isVisible()) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (m_pDlgFullSize->isVisible()) {
            m_pDlgFullSize->close();
        } else {
            m_pDlgFullSize->init(m_pLoadedTrack);
        }
    }
}
