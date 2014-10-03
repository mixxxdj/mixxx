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
          m_pCoverCache(CoverArtCache::instance()),
          m_defaultCover(scaledCoverArt(m_pCoverCache->getDefaultCoverArt())),
          m_bEnableWidget(true),
          m_pMenu(new WCoverArtMenu(this)),
          m_loadedCover(m_defaultCover),
          m_trackDAO(pTrackCollection->getTrackDAO()) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    connect(m_pCoverCache, SIGNAL(pixmapFound(int, QPixmap)),
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
    if (!m_pCoverCache->changeCoverArt(m_lastRequestedCover.trackId, newLoc)) {
        // parent must be NULL - it ensures the use of the default style.
        QMessageBox::warning(NULL, tr("Change Cover Art"),
                             tr("Could not change the cover art."));
    }
}

void WCoverArt::slotEnableWidget(bool enable) {
    m_bEnableWidget = enable;
    int h = (float) parentWidget()->height() / 3;
    h = m_bEnableWidget ? h : 0;
    setMinimumHeight(h);
    setMaximumHeight(h);
    update();
}

void WCoverArt::slotResetWidget() {
    m_lastRequestedCover = CoverInfo();
    m_loadedCover = m_defaultCover;
    update();
}

void WCoverArt::slotPixmapFound(int trackId, QPixmap pixmap) {
    if (!m_bEnableWidget) {
        return;
    }
    if (m_lastRequestedCover.trackId == trackId) {
        m_loadedCover = scaledCoverArt(pixmap);
        update();
    }
}

void WCoverArt::slotLoadCoverArt(CoverInfo info, bool cachedOnly) {
    if (!m_bEnableWidget) {
        return;
    }
    m_lastRequestedCover = info;
    m_pCoverCache->requestPixmap(info, QSize(0,0), cachedOnly);
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
    int x = width() / 2 - height() / 2 + 4;
    int y = 6;
    painter.drawPixmap(x, y, m_loadedCover);
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
        m_loadedCover = m_defaultCover;
    }
}

void WCoverArt::mousePressEvent(QMouseEvent* event) {
    if (!m_bEnableWidget) {
        return;
    }

    if (event->button() == Qt::RightButton) { // show context-menu
        TrackPointer pTrack = m_trackDAO.getTrack(m_lastRequestedCover.trackId);
        m_pMenu->show(event->globalPos(), m_lastRequestedCover, pTrack);
    } else if (event->button() == Qt::LeftButton) { // init/close fullsize cover
        DlgCoverArtFullSize* dlgFullSize = DlgCoverArtFullSize::instance();
        if (dlgFullSize->isVisible()) {
            dlgFullSize->close();
        } else {
            dlgFullSize->init(m_lastRequestedCover);
        }
    }
}

void WCoverArt::leaveEvent(QEvent*) {
        DlgCoverArtFullSize::instance()->close();
}
