#include "wcoverartlabel.h"

WCoverArtLabel::WCoverArtLabel(QWidget* parent)
        : QLabel(parent),
          m_pTrack(TrackPointer()),
          m_coverInfo(CoverInfo()),
          m_pCoverMenu(new WCoverArtMenu(this)) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setFrameShape(QFrame::Box);
    setAlignment(Qt::AlignCenter);
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

void WCoverArtLabel::setCoverArt(TrackPointer track, CoverInfo info, QPixmap px) {
    m_pTrack = track;
    m_coverInfo = info;
    setPixmap(px.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QSize frameSize = px.size();
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

void WCoverArtLabel::mousePressEvent(QMouseEvent* event) {
    if (m_pCoverMenu->isVisible()) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        DlgCoverArtFullSize* dlgFullSize = DlgCoverArtFullSize::instance();
        if (dlgFullSize->isVisible()) {
            dlgFullSize->close();
        } else {
            dlgFullSize->init(m_coverInfo);
        }
    }
}

void WCoverArtLabel::leaveEvent(QEvent*) {
    DlgCoverArtFullSize::instance()->close();
}
