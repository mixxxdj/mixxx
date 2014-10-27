#include <QAction>
#include <QApplication>
#include <QBitmap>
#include <QLabel>
#include <QIcon>
#include <QPainter>

#include "widget/wcoverart.h"
#include "widget/wskincolor.h"
#include "library/coverartcache.h"
#include "library/coverartutils.h"

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
                this, SLOT(slotPixmapFound(int, QPixmap)));
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

    if (context.hasNode(node, "DefaultCover")) {
        m_defaultCover = QPixmap(context.selectString(node, "DefaultCover"));
    }

    // If no default cover is specified or we failed to load it, fall back on
    // the resource bundle default cover.
    if (m_defaultCover.isNull()) {
        m_defaultCover = QPixmap(CoverArtUtils::defaultCoverLocation());
    }
    m_defaultCoverScaled = scaledCoverArt(m_defaultCover);
}

void WCoverArt::slotCoverLocationUpdated(const QString& newLoc,
                                         const QString& oldLoc,
                                         QPixmap newCover) {
    Q_UNUSED(oldLoc);
    Q_UNUSED(newCover);
    TrackPointer pTrack = m_trackDAO.getTrack(m_lastRequestedCover.trackId);
    if (pTrack) {
        CoverArt art;
        art.info.coverLocation = newLoc;
        art.info.source = CoverInfo::USER_SELECTED;
        art.info.type = CoverInfo::FILE;
        pTrack->setCoverArt(art);
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
    m_loadedCover = QPixmap();
    m_loadedCoverScaled = QPixmap();
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
        pCache->requestCover(info, QSize(0,0), cachedOnly);
    }
}

QPixmap WCoverArt::scaledCoverArt(const QPixmap& normal) {
    if (normal.isNull()) {
        return QPixmap();
    }

    int height = parentWidget()->height() / 3;
    return normal.scaled(QSize(height - 16, width() - 10),
                         Qt::KeepAspectRatio,
                         Qt::SmoothTransformation);
}

void WCoverArt::paintEvent(QPaintEvent* pEvent) {
    if (!m_bEnable) {
        QWidget::paintEvent(pEvent);
        return;
    }

    QPainter painter(this);

    QPixmap toDraw = m_loadedCoverScaled;
    if (toDraw.isNull()) {
        toDraw = m_defaultCoverScaled;
    }

    if (!toDraw.isNull()) {
        int x = 3 + width() / 2 - toDraw.width() / 2;
        int y = 8;
        painter.drawPixmap(x, y, toDraw);
    }

    QPen pen = painter.pen();
    pen.setColor(QColor("#656565"));
    painter.setPen(pen);
    painter.drawRoundedRect(5, 5, width() - 7, height() - 10, 0, 0);
}

void WCoverArt::resizeEvent(QResizeEvent*) {
    int h = (float) parentWidget()->height() / 3;
    if (height() && height() != h) {
        setMinimumHeight(h);
        setMaximumHeight(h);
    }
    m_loadedCoverScaled = scaledCoverArt(m_loadedCover);
    m_defaultCoverScaled = scaledCoverArt(m_defaultCover);
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
