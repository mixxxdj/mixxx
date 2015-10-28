#include <QtDebug>

#include "widget/wcoverartlabel.h"

#include "library/coverartutils.h"

static const QSize s_labelDisplaySize = QSize(100, 100);

WCoverArtLabel::WCoverArtLabel(QWidget* parent)
        : QLabel(parent),
          m_pCoverMenu(new WCoverArtMenu(this)),
          m_pDlgFullSize(new DlgCoverArtFullSize()),
          m_defaultCover(CoverArtUtils::defaultCoverLocation()) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setFrameShape(QFrame::Box);
    setAlignment(Qt::AlignCenter);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotCoverMenu(QPoint)));
    connect(m_pCoverMenu, SIGNAL(coverArtSelected(const CoverArt&)),
            this, SIGNAL(coverArtSelected(const CoverArt&)));
    connect(m_pCoverMenu, SIGNAL(reloadCoverArt()),
            this, SIGNAL(reloadCoverArt()));

    m_defaultCover = m_defaultCover.scaled(s_labelDisplaySize,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);
    setPixmap(m_defaultCover);
}

WCoverArtLabel::~WCoverArtLabel() {
    delete m_pCoverMenu;
    delete m_pDlgFullSize;
}

void WCoverArtLabel::setCoverArt(const QString& trackLocation, const CoverInfo& coverInfo, QPixmap px) {
    qDebug() << "WCoverArtLabel::setCoverArt" << coverInfo << px.size();

    m_coverInfo = coverInfo;
    m_pCoverMenu->setCoverArt(trackLocation, coverInfo);

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
