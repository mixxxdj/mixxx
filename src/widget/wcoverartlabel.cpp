#include "widget/wcoverartlabel.h"

#include "library/coverartutils.h"

static const QSize s_labelDisplaySize = QSize(100, 100);

WCoverArtLabel::WCoverArtLabel(QWidget* parent)
        : QLabel(parent),
          m_pTrack(TrackPointer()),
          m_coverInfo(CoverInfo()),
          m_pCoverMenu(new WCoverArtMenu(this)),
          m_pDlgFullSize(new DlgCoverArtFullSize()),
          m_defaultCover(CoverArtUtils::defaultCoverLocation()) {
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

    m_defaultCover = m_defaultCover.scaled(s_labelDisplaySize,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);
    setPixmap(m_defaultCover);
}

WCoverArtLabel::~WCoverArtLabel() {
    delete m_pCoverMenu;
    delete m_pDlgFullSize;
}

void WCoverArtLabel::setCoverArt(TrackPointer track, CoverInfo info, QPixmap px) {
    m_pTrack = track;
    m_coverInfo = info;

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
        if (m_pDlgFullSize->isVisible()) {
            m_pDlgFullSize->close();
        } else {
            m_pDlgFullSize->init(m_coverInfo);
        }
    }
}

void WCoverArtLabel::leaveEvent(QEvent*) {
    m_pDlgFullSize->close();
}
