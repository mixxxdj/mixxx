#include "widget/wcoverartlabel.h"

#include <QtDebug>

#include "library/coverartutils.h"
#include "library/dlgcoverartfullsize.h"
#include "moc_wcoverartlabel.cpp"
#include "track/track.h"
#include "util/compatibility.h"
#include "widget/wcoverartmenu.h"

namespace {

constexpr QSize kDeviceIndependentCoverLabelSize = QSize(100, 100);

inline QPixmap scaleCoverLabel(
        QWidget* parent,
        QPixmap pixmap) {
    const auto devicePixelRatioF = getDevicePixelRatioF(parent);
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

WCoverArtLabel::WCoverArtLabel(QWidget* parent)
        : QLabel(parent),
          m_pCoverMenu(make_parented<WCoverArtMenu>(this)),
          m_pDlgFullSize(make_parented<DlgCoverArtFullSize>(this)),
          m_defaultCover(createDefaultCover(this)) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setFrameShape(QFrame::Box);
    setAlignment(Qt::AlignCenter);
    connect(m_pCoverMenu,
            &WCoverArtMenu::coverInfoSelected,
            this,
            &WCoverArtLabel::coverInfoSelected);
    connect(m_pCoverMenu, &WCoverArtMenu::reloadCoverArt, this, &WCoverArtLabel::reloadCoverArt);

    setPixmap(m_defaultCover);
}

WCoverArtLabel::~WCoverArtLabel() = default;

void WCoverArtLabel::setCoverArt(const CoverInfo& coverInfo,
        const QPixmap& px) {
    m_pCoverMenu->setCoverArt(coverInfo);
    if (px.isNull()) {
        m_loadedCover = px;
        setPixmap(m_defaultCover);
    } else {
        m_loadedCover = scaleCoverLabel(this, px);
        setPixmap(m_loadedCover);
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    QSize frameSize = pixmap(Qt::ReturnByValue).size() / getDevicePixelRatioF(this);
#else
    QSize frameSize = pixmap()->size() / getDevicePixelRatioF(this);
#endif
    frameSize += QSize(2,2); // margin
    setMinimumSize(frameSize);
    setMaximumSize(frameSize);
}

void WCoverArtLabel::slotCoverMenu(const QPoint& pos) {
    m_pCoverMenu->popup(mapToGlobal(pos));
}

void WCoverArtLabel::contextMenuEvent(QContextMenuEvent* event) {
    event->accept();
    m_pCoverMenu->popup(event->globalPos());
}

void WCoverArtLabel::loadTrack(TrackPointer pTrack) {
    m_pLoadedTrack = pTrack;
}

void WCoverArtLabel::mousePressEvent(QMouseEvent* event) {
    if (m_pCoverMenu->isVisible()) {
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
