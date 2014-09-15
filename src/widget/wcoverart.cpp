#include <QAction>
#include <QApplication>
#include <QBitmap>
#include <QLabel>
#include <QIcon>
#include <QPainter>

#include "dlgcoverartfullsize.h"
#include "wcoverart.h"
#include "wskincolor.h"
#include "library/coverartcache.h"

WCoverArt::WCoverArt(QWidget* parent,
                     TrackCollection* pTrackCollection)
        : QWidget(parent),
          WBaseWidget(this),
          m_bEnableWidget(true),
          m_pMenu(new WCoverArtMenu(this)),
          m_loadedCover(CoverArtCache::instance()->getDefaultCoverArt()),
          m_loadedCoverScaled(scaledCoverArt(m_loadedCover)),
          m_trackDAO(pTrackCollection->getTrackDAO()) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    connect(CoverArtCache::instance(), SIGNAL(pixmapFound(int, QPixmap)),
            this, SLOT(slotPixmapFound(int, QPixmap)), Qt::DirectConnection);
    connect(m_pMenu,
            SIGNAL(coverLocationUpdated(const QString&, const QString&, QPixmap)),
            this,
            SLOT(slotCoverLocationUpdated(const QString&, const QString&, QPixmap)));
}

WCoverArt::~WCoverArt() {
    delete m_pMenu;
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
    bool res = CoverArtCache::instance()->changeCoverArt(
                    m_lastRequestedCover.trackId, newLoc);
    if (!res) {
        QMessageBox::warning(this, tr("Change Cover Art"),
                             tr("Could not change the cover art."));
    }
}

void WCoverArt::slotEnableWidget(bool enable) {
    m_bEnableWidget = enable;
    setMaximumHeight(m_bEnableWidget ? parentWidget()->height() / 3 : 0);
    update();
}

void WCoverArt::slotResetWidget() {
    m_lastRequestedCover = CoverInfo();
    m_loadedCover = CoverArtCache::instance()->getDefaultCoverArt();
    m_loadedCoverScaled = scaledCoverArt(m_loadedCover);
    update();
}

void WCoverArt::slotPixmapFound(int trackId, QPixmap pixmap) {
    if (!m_bEnableWidget) {
        return;
    }
    if (m_lastRequestedCover.trackId == trackId) {
        m_loadedCover = pixmap;
        m_loadedCoverScaled = scaledCoverArt(m_loadedCover);
        update();
    }
}

void WCoverArt::slotLoadCoverArt(CoverInfo info, bool cachedOnly) {
    if (!m_bEnableWidget) {
        return;
    }
    m_lastRequestedCover = info;
    CoverArtCache::instance()->requestPixmap(info, QSize(0,0), cachedOnly);
}

QPixmap WCoverArt::scaledCoverArt(QPixmap normal) {
    int height = parentWidget()->height() / 3;
    return normal.scaled(QSize(height - 10, height - 10),
                         Qt::KeepAspectRatio,
                         Qt::SmoothTransformation);
}

void WCoverArt::paintEvent(QPaintEvent*) {
    if (!m_bEnableWidget) {
        return;
    }
    QPainter painter(this);
    painter.drawLine(0, 0, width(), 0);
    int x = width() / 2 - height() / 2 + 4;
    int y = 6;
    painter.drawPixmap(x, y, m_loadedCoverScaled);
}

void WCoverArt::resizeEvent(QResizeEvent*) {
    if (height() && height() != parentWidget()->height() / 3) {
        setMaximumHeight(parentWidget()->height() / 3);
    }
    if (m_lastRequestedCover.trackId < 1) {
        m_loadedCover = CoverArtCache::instance()->getDefaultCoverArt();
        m_loadedCoverScaled = scaledCoverArt(m_loadedCover);
    }
}

void WCoverArt::mousePressEvent(QMouseEvent* event) {
    if (!m_bEnableWidget) {
        return;
    }
    // show context-menu
    if (event->button() == Qt::RightButton) {
        TrackPointer pTrack = m_trackDAO.getTrack(m_lastRequestedCover.trackId);
        m_pMenu->show(event->globalPos(), m_lastRequestedCover, pTrack);
    }
}

void WCoverArt::mouseMoveEvent(QMouseEvent* event) {
    if (event->HoverEnter) {
        DlgCoverArtFullSize::instance()->init(m_loadedCover);
    }
}

void WCoverArt::leaveEvent(QEvent*) {
        DlgCoverArtFullSize::instance()->close();
}
