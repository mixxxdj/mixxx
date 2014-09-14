#include "wcoverartlabel.h"

WCoverArtLabel::WCoverArtLabel(QWidget* parent)
        : QLabel(parent),
          m_coverInfo(CoverInfo()),
          m_pCoverMenu(new WCoverArtMenu(this)) {
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotCoverMenu(QPoint)));
    connect(m_pCoverMenu,
            SIGNAL(coverLocationUpdated(const QString&, const QString&, QPixmap)),
            this,
            SIGNAL(coverLocationUpdated(const QString&, const QString&, QPixmap)));
}

WCoverArtLabel::~WCoverArtLabel() {
    delete m_pCoverMenu;
}

void WCoverArtLabel::setCoverArt(TrackPointer track,
                                 CoverInfo info,
                                 QPixmap pixmap) {
    m_pTrack = track;
    m_coverInfo = info;
    QPixmap scaled = pixmap.scaled(100, 100,
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
    setPixmap(scaled);
    QSize frameSize = scaled.size();
    frameSize += QSize(2,2); // margin
    setMinimumSize(frameSize);
    setMaximumSize(frameSize);
}

void WCoverArtLabel::slotCoverMenu(const QPoint& pos) {
    if (m_pTrack == NULL) {
        return;
    }
    m_pCoverMenu->show(mapToGlobal(pos), m_coverInfo, m_pTrack);
}

void WCoverArtLabel::enterEvent(QEvent*) {
    if (!m_pCoverMenu->isVisible()) {
        DlgCoverArtFullSize::instance()->init(m_coverInfo);
    }
}

void WCoverArtLabel::leaveEvent(QEvent*) {
    DlgCoverArtFullSize::instance()->close();
}
