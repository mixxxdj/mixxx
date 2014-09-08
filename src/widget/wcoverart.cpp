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
                     ConfigObject<ConfigValue>* pConfig,
                     TrackCollection* pTrackCollection)
        : QWidget(parent),
          WBaseWidget(this),
          m_pConfig(pConfig),
          m_bEnableWidget(true),
          m_bCoverIsHovered(false),
          m_bCoverIsVisible(false),
          m_pMenu(new WCoverArtMenu(this)),
          m_loadedCover(CoverArtCache::instance()->getDefaultCoverArt()),
          m_lastRequestedTrackId(-1),
          m_trackDAO(pTrackCollection->getTrackDAO()) {
    // load icon to hide cover
    m_iconHide = QPixmap(":/images/library/ic_library_cover_hide.png");
    m_iconHide = m_iconHide.scaled(20,
                                   20,
                                   Qt::KeepAspectRatioByExpanding,
                                   Qt::SmoothTransformation);

    // load icon to show cover
    m_iconShow = QPixmap(":/images/library/ic_library_cover_show.png");
    m_iconShow = m_iconShow.scaled(17,
                                   17,
                                   Qt::KeepAspectRatioByExpanding,
                                   Qt::SmoothTransformation);

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

void WCoverArt::slotCoverLocationUpdated(const QString& newLoc,
                                         const QString& oldLoc,
                                         QPixmap px) {
    Q_UNUSED(oldLoc);
    Q_UNUSED(px);
    bool res = CoverArtCache::instance()->changeCoverArt(m_lastRequestedTrackId,
                                                         newLoc);
    if (!res) {
        QMessageBox::warning(this, tr("Change Cover Art"),
                             tr("Could not change the cover art."));
    }
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

void WCoverArt::slotEnableWidget(bool enable) {
    m_bEnableWidget = enable;
    setMinimumSize(0, 0);
    update();
}

void WCoverArt::slotResetWidget() {
    m_lastRequestedTrackId = -1;
    m_lastRequestedCover = qMakePair(QString(), QString());
    m_bCoverIsVisible = false;
    m_bCoverIsHovered = false;
    m_loadedCover = CoverArtCache::instance()->getDefaultCoverArt();
    m_loadedCover = scaledCoverArt(m_loadedCover);
    setMinimumSize(0, 20);
    update();
}

void WCoverArt::slotPixmapFound(int trackId, QPixmap pixmap) {
    if (!m_bEnableWidget) {
        return;
    }
    if (m_lastRequestedTrackId == trackId) {
        m_loadedCover = scaledCoverArt(pixmap);
        update();
    }
}

void WCoverArt::slotLoadCoverArt(const QString& coverLocation,
                                 const QString& md5Hash,
                                 int trackId, bool cachedOnly) {
    if (!m_bEnableWidget || !m_bCoverIsVisible) {
        return;
    }

    m_lastRequestedTrackId = trackId;
    m_lastRequestedCover = qMakePair(coverLocation, md5Hash);

    CoverArtCache::instance()->requestPixmap(trackId,
                                             coverLocation,
                                                             md5Hash,
                                                             QSize(0,0),
                                                             cachedOnly);
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

    if (m_bCoverIsVisible) {
        int x = width() / 2 - height() / 2 + 4;
        int y = 6;
        painter.drawPixmap(x, y, m_loadedCover);
    } else {
        painter.drawPixmap(1, 2 ,m_iconShow);
        painter.drawText(25, 15, tr("Show Cover Art"));
    }

    if (m_bCoverIsVisible && m_bCoverIsHovered) {
        painter.drawPixmap(width() - 21, 6, m_iconHide);
    }
}

void WCoverArt::resizeEvent(QResizeEvent*) {
    if (!m_bEnableWidget) {
        setMinimumSize(0, 0);
        return;
    }

    if (m_bCoverIsVisible) {
        setMinimumSize(0, parentWidget()->height() / 3);
        slotLoadCoverArt(m_lastRequestedCover.first,
                         m_lastRequestedCover.second,
                         m_lastRequestedTrackId, true);
     } else {
        m_loadedCover = CoverArtCache::instance()->getDefaultCoverArt();
        setMinimumSize(0, 20);
        DlgCoverArtFullSize::instance()->close();
    }
}

void WCoverArt::mousePressEvent(QMouseEvent* event) {
    if (!m_bEnableWidget) {
        return;
    }

    if (!m_bCoverIsVisible) { // show widget
        m_bCoverIsVisible = true;
        resize(sizeHint());
        return;
    }

    QPoint lastPoint(event->pos());
    if(lastPoint.x() > width() - (height() / 5)
            && lastPoint.y() < (height() / 5) + 5) { // hide widget
        m_bCoverIsVisible = false;
        resize(sizeHint());
    } else if (event->button() == Qt::RightButton) { // show context-menu
        TrackPointer pTrack = m_trackDAO.getTrack(m_lastRequestedTrackId);
        m_pMenu->show(event->globalPos(),
                      m_lastRequestedCover,
                      m_lastRequestedTrackId,
                      pTrack);
    }
}

void WCoverArt::mouseMoveEvent(QMouseEvent* event) {
    m_bCoverIsHovered  = event->HoverEnter;
    update();
}

void WCoverArt::leaveEvent(QEvent*) {
    m_bCoverIsHovered = false;
    update();
}
