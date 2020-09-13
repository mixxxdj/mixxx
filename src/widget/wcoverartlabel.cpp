#include "widget/wcoverartlabel.h"

#include <QtDebug>

#include "library/dlgcoverartfullsize.h"
#include "library/coverartutils.h"
#include "util/compatibility.h"

static const QSize s_labelDisplaySize = QSize(100, 100);

WCoverArtLabel::WCoverArtLabel(QWidget* parent)
        : QLabel(parent),
          m_pCoverMenu(make_parented<WCoverArtMenu>(this)),
          m_pDlgFullSize(make_parented<DlgCoverArtFullSize>(this)),
          m_defaultCover(CoverArtUtils::defaultCoverLocation()) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setFrameShape(QFrame::Box);
    setAlignment(Qt::AlignCenter);
    connect(m_pCoverMenu,
            &WCoverArtMenu::coverInfoSelected,
            this,
            &WCoverArtLabel::coverInfoSelected);
    connect(m_pCoverMenu, &WCoverArtMenu::reloadCoverArt, this, &WCoverArtLabel::reloadCoverArt);

    m_defaultCover.setDevicePixelRatio(getDevicePixelRatioF(this));
    m_defaultCover = m_defaultCover.scaled(s_labelDisplaySize * getDevicePixelRatioF(this),
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);
    setPixmap(m_defaultCover);
}

WCoverArtLabel::~WCoverArtLabel() {
    // Nothing to do
}

void WCoverArtLabel::setCoverArt(const CoverInfo& coverInfo,
                                 QPixmap px) {
    m_pCoverMenu->setCoverArt(coverInfo);
    if (px.isNull()) {
        m_loadedCover = px;
        setPixmap(m_defaultCover);
    } else {
        m_loadedCover = px.scaled(s_labelDisplaySize * getDevicePixelRatioF(this),
                Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_loadedCover.setDevicePixelRatio(getDevicePixelRatioF(this));
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
