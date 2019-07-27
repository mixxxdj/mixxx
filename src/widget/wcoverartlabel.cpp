#include "widget/wcoverartlabel.h"

#include <QtDebug>

#include "library/dlgcoverartfullsize.h"
#include "library/coverartutils.h"

static const QSize s_labelDisplaySize = QSize(100, 100);

WCoverArtLabel::WCoverArtLabel(QWidget* parent)
        : QLabel(parent),
          m_pCoverMenu(make_parented<WCoverArtMenu>(this)),
          m_pDlgFullSize(make_parented<DlgCoverArtFullSize>(this)),
          m_defaultCover(CoverArtUtils::defaultCoverLocation()) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setFrameShape(QFrame::Box);
    setAlignment(Qt::AlignCenter);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotCoverMenu(QPoint)));
    connect(m_pCoverMenu, SIGNAL(coverInfoSelected(const CoverInfoRelative&)),
            this, SIGNAL(coverInfoSelected(const CoverInfoRelative&)));
    connect(m_pCoverMenu, SIGNAL(reloadCoverArt()),
            this, SIGNAL(reloadCoverArt()));

    m_defaultCover = m_defaultCover.scaled(s_labelDisplaySize,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);
    setPixmap(m_defaultCover);
}

WCoverArtLabel::~WCoverArtLabel() {
    // Nothing to do
}

void WCoverArtLabel::setCoverArt(const CoverInfo& coverInfo,
                                 QPixmap px) {
    qDebug() << "WCoverArtLabel::setCoverArt" << coverInfo << px.size();

    m_loadedCover = px;
    m_pCoverMenu->setCoverArt(coverInfo);


    if (px.isNull()) {
        setPixmap(m_defaultCover);
    } else {
        setPixmap(px.scaled(s_labelDisplaySize, Qt::KeepAspectRatio,
                            Qt::SmoothTransformation));
    }

    QSize frameSize = pixmap()->size();
    frameSize += QSize(2,2); // margin
    setMinimumSize(frameSize);
    setMaximumSize(frameSize);
}

void WCoverArtLabel::slotCoverMenu(const QPoint& pos) {
    m_pCoverMenu->popup(mapToGlobal(pos));
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

