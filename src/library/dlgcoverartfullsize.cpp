#include "library/dlgcoverartfullsize.h"

#include <QRect>
#include <QScreen>
#include <QStyle>
#include <QWheelEvent>

#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "track/track.h"
#include "util/widgethelper.h"

DlgCoverArtFullSize::DlgCoverArtFullSize(QWidget* parent, BaseTrackPlayer* pPlayer)
        : QDialog(parent),
          m_pPlayer(pPlayer),
          m_pCoverMenu(make_parented<WCoverArtMenu>(this)) {
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache) {
        connect(pCache,
                &CoverArtCache::coverFound,
                this,
                &DlgCoverArtFullSize::slotCoverFound);
    }

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,
            &DlgCoverArtFullSize::customContextMenuRequested,
            this,
            &DlgCoverArtFullSize::slotCoverMenu);
    connect(m_pCoverMenu,
            &WCoverArtMenu::coverInfoSelected,
            this,
            &DlgCoverArtFullSize::slotCoverInfoSelected);
    connect(m_pCoverMenu,
            &WCoverArtMenu::reloadCoverArt,
            this,
            &DlgCoverArtFullSize::slotReloadCoverArt);

    if (m_pPlayer != nullptr) {
        connect(pPlayer,
                &BaseTrackPlayer::newTrackLoaded,
                this,
                &DlgCoverArtFullSize::slotLoadTrack);
    }

    setupUi(this);
}

void DlgCoverArtFullSize::closeEvent(QCloseEvent* event) {
    if (parentWidget()) {
        // Since the widget has a parent, this instance will be reused again.
        // We need to prevent qt from destroying it's children
        hide();
        slotLoadTrack(nullptr);
        event->ignore();
    } else {
        QDialog::closeEvent(event);
    }
}

void DlgCoverArtFullSize::init(TrackPointer pTrack) {
    if (!pTrack) {
        return;
    }
    // The real size will be calculated later.
    // If you zoom in so the window is larger then the desktop, close the
    // window and reopen, the window will not be resized correctly
    // by slotCoverFound. Setting a small fixed size before show fixes this
    resize(100, 100);
    show();
    raise();
    activateWindow();

    // This must be called after show() to set the window title. Refer to the
    // comment in slotLoadTrack for details.
    slotLoadTrack(pTrack);
}

void DlgCoverArtFullSize::slotLoadTrack(TrackPointer pTrack) {
    if (m_pLoadedTrack != nullptr) {
        disconnect(m_pLoadedTrack.get(),
                &Track::coverArtUpdated,
                this,
                &DlgCoverArtFullSize::slotTrackCoverArtUpdated);
    }
    m_pLoadedTrack = pTrack;
    if (m_pLoadedTrack != nullptr) {
        connect(m_pLoadedTrack.get(),
                &Track::coverArtUpdated,
                this,
                &DlgCoverArtFullSize::slotTrackCoverArtUpdated);

        // Somehow setting the widow title triggered a bug in Xlib that resulted
        // in a deadlock before the check for isVisible() was added.
        // Unfortunately the original bug was difficult to reproduce, so I am
        // not sure if checking isVisible() before setting the window title
        // actually works around the Xlib bug or merely makes it much less
        // likely to be triggered. Before the isVisible() check was added,
        // the window title was getting set on DlgCoverArtFullSize instances
        // that had never been shown whenever a track was loaded.
        // https://bugs.launchpad.net/mixxx/+bug/1789059
        // https://gitlab.freedesktop.org/xorg/lib/libx11/issues/25#note_50985
        if (isVisible()) {
            QString windowTitle;
            const QString albumArtist = m_pLoadedTrack->getAlbumArtist();
            const QString artist = m_pLoadedTrack->getArtist();
            const QString album = m_pLoadedTrack->getAlbum();
            const QString year = m_pLoadedTrack->getYear();
            if (!albumArtist.isEmpty()) {
                windowTitle = albumArtist;
            } else if (!artist.isEmpty()) {
                windowTitle += artist;
            }
            if (!album.isEmpty()) {
                if (!windowTitle.isEmpty()) {
                    windowTitle += " - ";
                }
                windowTitle += album;
            }
            if (!year.isEmpty()) {
                if (!windowTitle.isEmpty()) {
                    windowTitle += " ";
                }
                windowTitle += QString("(%1)").arg(year);
            }
            setWindowTitle(windowTitle);
        }
    }
    slotTrackCoverArtUpdated();
}

void DlgCoverArtFullSize::slotTrackCoverArtUpdated() {
    if (m_pLoadedTrack) {
        CoverArtCache::requestTrackCover(this, m_pLoadedTrack);
    } else {
        coverArt->setPixmap(QPixmap());
    }
}

void DlgCoverArtFullSize::slotCoverFound(
        const QObject* pRequestor,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap,
        quint16 requestedHash,
        bool coverInfoUpdated) {
    Q_UNUSED(requestedHash);
    Q_UNUSED(coverInfoUpdated);
    if (pRequestor != this || !m_pLoadedTrack ||
            m_pLoadedTrack->getLocation() != coverInfo.trackLocation) {
        return;
    }

    m_pixmap = pixmap;

    if (m_pixmap.isNull()) {
        coverArt->setPixmap(QPixmap());
        hide();
        return;
    }

    // Scale down dialog if the pixmap is larger than the screen.
    // Use 90% of screen size instead of 100% to prevent an issue with
    // whitespace appearing on the side when resizing a window whose
    // borders touch the edges of the screen.
    QSize dialogSize = m_pixmap.size();
    QWidget* centerOverWidget = parentWidget();
    VERIFY_OR_DEBUG_ASSERT(centerOverWidget) {
        qWarning() << "DlgCoverArtFullSize does not have a parent.";
        centerOverWidget = this;
    }

    const QScreen* const pScreen = mixxx::widgethelper::getScreen(*centerOverWidget);
    QRect screenGeometry;
    VERIFY_OR_DEBUG_ASSERT(pScreen) {
        qWarning() << "Assuming screen size of 800x600px.";
        screenGeometry = QRect(0, 0, 800, 600);
    }
    else {
        screenGeometry = pScreen->geometry();
    }

    const QSize availableScreenSpace = screenGeometry.size() * 0.9;
    if (dialogSize.height() > availableScreenSpace.height()) {
        dialogSize.scale(dialogSize.width(), screenGeometry.height(), Qt::KeepAspectRatio);
    } else if (dialogSize.width() > screenGeometry.width()) {
        dialogSize.scale(screenGeometry.width(), dialogSize.height(), Qt::KeepAspectRatio);
    }
    QPixmap resizedPixmap = m_pixmap.scaled(size() * getDevicePixelRatioF(this),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
    resizedPixmap.setDevicePixelRatio(getDevicePixelRatioF(this));
    coverArt->setPixmap(resizedPixmap);

    // center the window
    setGeometry(QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            dialogSize,
            screenGeometry));
}

// slots to handle signals from the context menu
void DlgCoverArtFullSize::slotReloadCoverArt() {
    if (!m_pLoadedTrack) {
        return;
    }
    slotCoverInfoSelected(
            CoverInfoGuesser().guessCoverInfoForTrack(
                    *m_pLoadedTrack));
}

void DlgCoverArtFullSize::slotCoverInfoSelected(
        const CoverInfoRelative& coverInfo) {
    if (!m_pLoadedTrack) {
        return;
    }
    m_pLoadedTrack->setCoverInfo(coverInfo);
}

void DlgCoverArtFullSize::mousePressEvent(QMouseEvent* event) {
    if (!m_pCoverMenu->isVisible() && event->button() == Qt::LeftButton) {
        m_clickTimer.setSingleShot(true);
        m_clickTimer.start(500);
        m_coverPressed = true;
        m_dragStartPosition = event->globalPos() - frameGeometry().topLeft();
    }
}

void DlgCoverArtFullSize::mouseReleaseEvent(QMouseEvent* event) {
    m_coverPressed = false;
    if (m_pCoverMenu->isVisible()) {
        return;
    }

    if (event->button() == Qt::LeftButton && isVisible()) {
        if (m_clickTimer.isActive()) {
        // short press
            close();
        } else {
        // long press
            return;
        }
        event->accept();
    }
}

void DlgCoverArtFullSize::mouseMoveEvent(QMouseEvent* event) {
    if (m_coverPressed) {
        move(event->globalPos() - m_dragStartPosition);
        event->accept();
    } else {
        return;
    }
}

void DlgCoverArtFullSize::slotCoverMenu(const QPoint& pos) {
    m_pCoverMenu->popup(mapToGlobal(pos));
}

void DlgCoverArtFullSize::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);
    if (m_pixmap.isNull()) {
        return;
    }
    // qDebug() << "DlgCoverArtFullSize::resizeEvent" << size();
    QPixmap resizedPixmap = m_pixmap.scaled(size() * getDevicePixelRatioF(this),
        Qt::KeepAspectRatio, Qt::SmoothTransformation);
    resizedPixmap.setDevicePixelRatio(getDevicePixelRatioF(this));
    coverArt->setPixmap(resizedPixmap);
}

void DlgCoverArtFullSize::wheelEvent(QWheelEvent* event) {
    // Scale the image size
    int oldWidth = width();
    int oldHeight = height();
    auto newWidth = static_cast<int>(oldWidth + (0.2 * event->angleDelta().y()));
    auto newHeight = static_cast<int>(oldHeight + (0.2 * event->angleDelta().y()));
    QSize newSize = size();
    newSize.scale(newWidth, newHeight, Qt::KeepAspectRatio);

    // To keep the same part of the image under the cursor, shift the
    // origin (top left point) by the distance the point moves under the cursor.
    QPoint oldOrigin = geometry().topLeft();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QPoint oldPointUnderCursor = event->position().toPoint();
#else
    QPoint oldPointUnderCursor = event->pos();
#endif

    const auto newPointX = static_cast<int>(
            static_cast<double>(oldPointUnderCursor.x()) / oldWidth * newSize.width());
    const auto newPointY = static_cast<int>(
            static_cast<double>(oldPointUnderCursor.y()) / oldHeight * newSize.height());
    QPoint newOrigin = QPoint(
        oldOrigin.x() + (oldPointUnderCursor.x() - newPointX),
        oldOrigin.y() + (oldPointUnderCursor.y() - newPointY));

    // Calling resize() then move() causes flickering, so resize and move the window
    // simultaneously with setGeometry().
    setGeometry(QRect(newOrigin, newSize));

    event->accept();
}
