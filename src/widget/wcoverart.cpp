#include <QAction>
#include <QApplication>
#include <QBitmap>
#include <QLabel>
#include <QIcon>
#include <QPainter>

#include "wcoverart.h"
#include "wskincolor.h"
#include "library/coverartcache.h"

WCoverArt::WCoverArt(QWidget* parent,
                     TrackCollection* pTrackCollection)
        : QWidget(parent),
          WBaseWidget(this),
          m_bEnable(true),
          m_pMenu(new WCoverArtMenu(this)),
          m_trackDAO(pTrackCollection->getTrackDAO()),
          m_pDlgFullSize(new DlgCoverArtFullSize()) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache != NULL) {
        connect(pCache, SIGNAL(pixmapFound(int, QPixmap)),
                this, SLOT(slotPixmapFound(int, QPixmap)), Qt::DirectConnection);
        m_loadedCover = pCache->getDefaultCoverArt();
        m_loadedCoverScaled = scaledCoverArt(m_loadedCover);
    }
    connect(m_pMenu,
            SIGNAL(coverLocationUpdated(const QString&, const QString&, QPixmap)),
            this,
            SLOT(slotCoverLocationUpdated(const QString&, const QString&, QPixmap)));
}

WCoverArt::~WCoverArt() {
    delete m_pMenu;
    delete m_pDlgFullSize;
}

void WCoverArt::setup(QDomNode node, const SkinContext& context) {
    Q_UNUSED(node);
    setMouseTracking(TRUE);

    // Background color
    QColor bgc(255,255,255);
    if (context.hasNode(node, "BgColor")) {
        bgc.setNamedColor(context.selectString(node, "BgColor"));
        setAutoFillBackground(true);
    }
    QPalette pal = palette();
    pal.setBrush(backgroundRole(), WSkinColor::getCorrectColor(bgc));

    // Foreground color
    QColor m_fgc(0,0,0);
    if (context.hasNode(node, "FgColor")) {
        m_fgc.setNamedColor(context.selectString(node, "FgColor"));
    }
    bgc = WSkinColor::getCorrectColor(bgc);
    m_fgc = QColor(255 - bgc.red(), 255 - bgc.green(), 255 - bgc.blue());
    pal.setBrush(foregroundRole(), m_fgc);
    setPalette(pal);
}

void WCoverArt::slotCoverLocationUpdated(const QString& newLoc,
                                         const QString& oldLoc,
                                         QPixmap px) {
    Q_UNUSED(oldLoc);
    Q_UNUSED(px);
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache != NULL && !pCache->changeCoverArt(
            m_lastRequestedCover.trackId, newLoc)) {
        // parent must be NULL - it ensures the use of the default style.
        QMessageBox::warning(NULL, tr("Change Cover Art"),
                             tr("Could not change the cover art."));
    }
}

void WCoverArt::slotEnable(bool enable) {
    m_bEnable = enable;
    int h = (float) parentWidget()->height() / 3;
    h = m_bEnable ? h : 0;
    setMinimumHeight(h);
    setMaximumHeight(h);
    update();
}

void WCoverArt::slotReset() {
    m_lastRequestedCover = CoverInfo();
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache != NULL) {
        m_loadedCover = pCache->getDefaultCoverArt();
        m_loadedCoverScaled = scaledCoverArt(m_loadedCover);
    }
    update();
}

void WCoverArt::slotPixmapFound(int trackId, QPixmap pixmap) {
    if (!m_bEnable) {
        return;
    }
    if (m_lastRequestedCover.trackId == trackId) {
        m_loadedCover = pixmap;
        m_loadedCoverScaled = scaledCoverArt(pixmap);
        update();
    }
}

void WCoverArt::slotLoadCoverArt(CoverInfo info, bool cachedOnly) {
    if (!m_bEnable) {
        return;
    }
    m_lastRequestedCover = info;
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache != NULL) {
        pCache->requestPixmap(info, QSize(0,0), cachedOnly);
    }
}

QPixmap WCoverArt::scaledCoverArt(QPixmap normal) {
    int height = parentWidget()->height() / 3;
    return normal.scaled(QSize(height - 16, width() - 10),
                         Qt::KeepAspectRatio,
                         Qt::SmoothTransformation);
}

void WCoverArt::paintEvent(QPaintEvent*) {
    if (!m_bEnable) {
        return;
    }

    QPainter painter(this);
    int x = 3 + width() / 2 - m_loadedCoverScaled.width() / 2;
    int y = 8;
    painter.drawPixmap(x, y, m_loadedCoverScaled);
    QPen pen = painter.pen();
    pen.setColor(QColor("#656565"));
    painter.setPen(pen);
    painter.drawRoundedRect(5, 5, width()-7, height()-10, 0, 0);
}

void WCoverArt::resizeEvent(QResizeEvent*) {
    int h = (float) parentWidget()->height() / 3;
    if (height() && height() != h) {
        setMinimumHeight(h);
        setMaximumHeight(h);
    }
    if (m_lastRequestedCover.trackId < 1) {
        CoverArtCache* pCache = CoverArtCache::instance();
        if (pCache != NULL) {
            m_loadedCover = pCache->getDefaultCoverArt();
        }
    }
    m_loadedCoverScaled = scaledCoverArt(m_loadedCover);
}

void WCoverArt::mousePressEvent(QMouseEvent* event) {
    if (!m_bEnable) {
        return;
    }

    if (event->button() == Qt::RightButton) { // show context-menu
        TrackPointer pTrack = m_trackDAO.getTrack(m_lastRequestedCover.trackId);
        m_pMenu->show(event->globalPos(), m_lastRequestedCover, pTrack);
    } else if (event->button() == Qt::LeftButton) { // init/close fullsize cover
        if (m_pDlgFullSize->isVisible()) {
            m_pDlgFullSize->close();
        } else {
            m_pDlgFullSize->init(m_lastRequestedCover);
        }
    }
}

void WCoverArt::leaveEvent(QEvent*) {
    m_pDlgFullSize->close();
}
