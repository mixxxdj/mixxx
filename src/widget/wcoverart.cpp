#include "widget/wcoverart.h"

#include <QAction>
#include <QApplication>
#include <QBitmap>
#include <QIcon>
#include <QLabel>
#include <QStyleOption>
#include <QStylePainter>

#include "control/controlobject.h"
#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "library/dlgcoverartfullsize.h"
#include "track/track.h"
#include "util/compatibility.h"
#include "util/dnd.h"
#include "util/math.h"
#include "widget/wskincolor.h"

WCoverArt::WCoverArt(QWidget* parent,
                     UserSettingsPointer pConfig,
                     const QString& group,
                     BaseTrackPlayer* pPlayer)
        : QWidget(parent),
          WBaseWidget(this),
          m_group(group),
          m_pConfig(pConfig),
          m_bEnable(true),
          m_pMenu(new WCoverArtMenu(this)),
          m_pPlayer(pPlayer),
          m_pDlgFullSize(new DlgCoverArtFullSize(this, pPlayer)) {
    // Accept drops if we have a group to load tracks into.
    setAcceptDrops(!m_group.isEmpty());

    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache) {
        connect(pCache,
                &CoverArtCache::coverFound,
                this,
                &WCoverArt::slotCoverFound);
    }
    connect(m_pMenu, SIGNAL(coverInfoSelected(const CoverInfoRelative&)),
            this, SLOT(slotCoverInfoSelected(const CoverInfoRelative&)));
    connect(m_pMenu, SIGNAL(reloadCoverArt()),
            this, SLOT(slotReloadCoverArt()));

    if (m_pPlayer != nullptr) {
        connect(m_pPlayer, SIGNAL(newTrackLoaded(TrackPointer)),
                this, SLOT(slotLoadTrack(TrackPointer)));
        connect(m_pPlayer, SIGNAL(loadingTrack(TrackPointer, TrackPointer)),
                this, SLOT(slotLoadingTrack(TrackPointer, TrackPointer)));

        // just in case a track is already loaded
        slotLoadTrack(m_pPlayer->getLoadedTrack());
    }
}

WCoverArt::~WCoverArt() {
    delete m_pMenu;
    delete m_pDlgFullSize;
}

void WCoverArt::setup(const QDomNode& node, const SkinContext& context) {
    Q_UNUSED(node);
    setMouseTracking(true);

    // Background color
    QColor bgc(255,255,255);
    QString bgColorStr;
    if (context.hasNodeSelectString(node, "BgColor", &bgColorStr)) {
        bgc.setNamedColor(bgColorStr);
        setAutoFillBackground(true);
    }
    QPalette pal = palette();
    pal.setBrush(backgroundRole(), WSkinColor::getCorrectColor(bgc));

    // Foreground color
    QColor m_fgc(0,0,0);
    QString fgColorStr;
    if (context.hasNodeSelectString(node, "FgColor", &fgColorStr)) {
        m_fgc.setNamedColor(fgColorStr);
    }
    bgc = WSkinColor::getCorrectColor(bgc);
    m_fgc = QColor(255 - bgc.red(), 255 - bgc.green(), 255 - bgc.blue());
    pal.setBrush(foregroundRole(), m_fgc);
    setPalette(pal);

    QString defaultCoverStr;
    if (context.hasNodeSelectString(node, "DefaultCover", &defaultCoverStr)) {
        m_defaultCover = QPixmap(defaultCoverStr);
    }

    // If no default cover is specified or we failed to load it, fall back on
    // the resource bundle default cover.
    if (m_defaultCover.isNull()) {
        m_defaultCover = QPixmap(CoverArtUtils::defaultCoverLocation());
    }
    m_defaultCoverScaled = scaledCoverArt(m_defaultCover);
}

void WCoverArt::slotReloadCoverArt() {
    if (!m_loadedTrack) {
        return;
    }
    guessTrackCoverInfoConcurrently(m_loadedTrack);
}

void WCoverArt::slotCoverInfoSelected(const CoverInfoRelative& coverInfo) {
    if (m_loadedTrack) {
        // Will trigger slotTrackCoverArtUpdated().
        m_loadedTrack->setCoverInfo(coverInfo);
    }
}

void WCoverArt::slotEnable(bool enable) {
    bool wasDisabled = !m_bEnable && enable;
    m_bEnable = enable;

    if (wasDisabled) {
        slotLoadTrack(m_loadedTrack);
    }
    update();
}

void WCoverArt::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    Q_UNUSED(pOldTrack);
    slotReset();
}

void WCoverArt::slotReset() {
    if (m_loadedTrack) {
        disconnect(
                m_loadedTrack.get(),
                &Track::coverArtUpdated,
                this,
                &WCoverArt::slotTrackCoverArtUpdated);
    }
    m_loadedTrack.reset();
    m_lastRequestedCover = CoverInfo();
    m_loadedCover = QPixmap();
    m_loadedCoverScaled = QPixmap();
    update();
}

void WCoverArt::slotTrackCoverArtUpdated() {
    if (m_loadedTrack) {
        CoverArtCache::requestTrackCover(this, m_loadedTrack);
    }
}

void WCoverArt::slotCoverFound(
        const QObject* pRequestor,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap,
        mixxx::cache_key_t requestedCacheKey,
        bool coverInfoUpdated) {
    Q_UNUSED(requestedCacheKey);
    Q_UNUSED(coverInfoUpdated);
    if (!m_bEnable) {
        return;
    }

    if (pRequestor == this &&
            m_loadedTrack &&
            m_loadedTrack->getLocation() == coverInfo.trackLocation) {
        m_lastRequestedCover = coverInfo;
        m_loadedCover = pixmap;
        m_loadedCoverScaled = scaledCoverArt(pixmap);
        update();
    }
}

void WCoverArt::slotLoadTrack(TrackPointer pTrack) {
    if (m_loadedTrack) {
        disconnect(m_loadedTrack.get(), SIGNAL(coverArtUpdated()),
                   this, SLOT(slotTrackCoverArtUpdated()));
    }
    m_lastRequestedCover = CoverInfo();
    m_loadedCover = QPixmap();
    m_loadedCoverScaled = QPixmap();
    m_loadedTrack = pTrack;
    if (m_loadedTrack) {
        connect(m_loadedTrack.get(),
                &Track::coverArtUpdated,
                this,
                &WCoverArt::slotTrackCoverArtUpdated);
    }

    if (!m_bEnable) {
        return;
    }

    slotTrackCoverArtUpdated();
}

QPixmap WCoverArt::scaledCoverArt(const QPixmap& normal) {
    if (normal.isNull()) {
        return QPixmap();
    }
    QPixmap scaled;
    scaled = normal.scaled(size() * getDevicePixelRatioF(this),
            Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaled.setDevicePixelRatio(getDevicePixelRatioF(this));
    return scaled;
}

void WCoverArt::paintEvent(QPaintEvent* /*unused*/) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter painter(this);
    painter.drawPrimitive(QStyle::PE_Widget, option);

    if (!m_bEnable) {
        return;
    }

    QPixmap toDraw = m_loadedCoverScaled;
    if (toDraw.isNull()) {
        toDraw = m_defaultCoverScaled;
    }

    if (!toDraw.isNull()) {
        QSize widgetSize = size();
        QSize pixmapSize = toDraw.size();

        int x = math_max(0, (widgetSize.width() - pixmapSize.width()) / 2);
        int y = math_max(0, (widgetSize.height() - pixmapSize.height()) / 2);
        painter.drawPixmap(x, y, toDraw);
    }
}

void WCoverArt::resizeEvent(QResizeEvent* /*unused*/) {
    m_loadedCoverScaled = scaledCoverArt(m_loadedCover);
    m_defaultCoverScaled = scaledCoverArt(m_defaultCover);
}

void WCoverArt::contextMenuEvent(QContextMenuEvent* event) {
    event->accept();
    if (m_loadedTrack) {
        m_pMenu->setCoverArt(m_lastRequestedCover);
        m_pMenu->popup(event->globalPos());
    }
}

void WCoverArt::mousePressEvent(QMouseEvent* event) {
    if (!m_bEnable) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        event->accept();
        // do nothing if left button is pressed,
        // wait for button release
        m_clickTimer.setSingleShot(true);
        m_clickTimer.start(500);
    }
}

void WCoverArt::mouseReleaseEvent(QMouseEvent* event) {
    if (!m_bEnable) {
        return;
    }

    if (event->button() == Qt::LeftButton && m_loadedTrack &&
            m_clickTimer.isActive()) { // init/close fullsize cover
        if (m_pDlgFullSize->isVisible()) {
            m_pDlgFullSize->close();
        } else if (!m_loadedCover.isNull()) {
            // Only show the fullsize cover art dialog if the current track
            // actually has a cover.  The `init` method already shows the
            // window and then emits a signal to load the cover, so this can't
            // be handled by the method itself.
            m_pDlgFullSize->init(m_loadedTrack);
        }
    } // else it was a long leftclick or a right click that's already been processed
}

void WCoverArt::mouseMoveEvent(QMouseEvent* event) {
    if ((event->buttons().testFlag(Qt::LeftButton)) && m_loadedTrack) {
        DragAndDropHelper::dragTrack(m_loadedTrack, this, m_group);
    }
}

void WCoverArt::dragEnterEvent(QDragEnterEvent* event) {
    // If group is empty then we are a library cover art widget and we don't
    // accept track drops.
    if (!m_group.isEmpty()) {
        DragAndDropHelper::handleTrackDragEnterEvent(event, m_group, m_pConfig);
    } else {
        event->ignore();
    }
}

void WCoverArt::dropEvent(QDropEvent *event) {
    // If group is empty then we are a library cover art widget and we don't
    // accept track drops.
    if (!m_group.isEmpty()) {
        DragAndDropHelper::handleTrackDropEvent(event, *this, m_group, m_pConfig);
    } else {
        event->ignore();
    }
}

bool WCoverArt::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QWidget::event(pEvent);
}
