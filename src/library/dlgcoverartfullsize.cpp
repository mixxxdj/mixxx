#include <QDesktopWidget>
#include <QStyle>
#include <QWheelEvent>

#include "library/dlgcoverartfullsize.h"
#include "library/coverartutils.h"
#include "library/coverartcache.h"

DlgCoverArtFullSize::DlgCoverArtFullSize(QWidget* parent, BaseTrackPlayer* pPlayer)
        : QDialog(parent),
          m_pPlayer(pPlayer),
          m_pCoverMenu(make_parented<WCoverArtMenu>(this)) {
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache != nullptr) {
        connect(pCache, SIGNAL(coverFound(const QObject*,
                                          const CoverInfoRelative&, QPixmap, bool)),
                this, SLOT(slotCoverFound(const QObject*,
                                          const CoverInfoRelative&, QPixmap, bool)));
    }

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotCoverMenu(QPoint)));
    connect(m_pCoverMenu, SIGNAL(coverInfoSelected(const CoverInfoRelative&)),
            this, SLOT(slotCoverInfoSelected(const CoverInfoRelative&)));
    connect(m_pCoverMenu, SIGNAL(reloadCoverArt()),
            this, SLOT(slotReloadCoverArt()));

    if (m_pPlayer != nullptr) {
        connect(pPlayer, SIGNAL(newTrackLoaded(TrackPointer)),
                this, SLOT(slotLoadTrack(TrackPointer)));
    }

    setupUi(this);
}

void DlgCoverArtFullSize::init(TrackPointer pTrack) {
    if (!pTrack) {
        return;
    }
    show();
    raise();
    activateWindow();

    // This must be called after show() to set the window title. Refer to the
    // comment in slotLoadTrack for details.
    slotLoadTrack(pTrack);
}

void DlgCoverArtFullSize::slotLoadTrack(TrackPointer pTrack) {
    if (m_pLoadedTrack != nullptr) {
        disconnect(m_pLoadedTrack.get(), SIGNAL(coverArtUpdated()),
                   this, SLOT(slotTrackCoverArtUpdated()));
    }
    m_pLoadedTrack = pTrack;
    if (m_pLoadedTrack != nullptr) {
        connect(m_pLoadedTrack.get(), SIGNAL(coverArtUpdated()),
                this, SLOT(slotTrackCoverArtUpdated()));

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
    if (m_pLoadedTrack != nullptr) {
        CoverArtCache::requestCover(*m_pLoadedTrack, this);
    }
}

void DlgCoverArtFullSize::slotCoverFound(const QObject* pRequestor,
                                         const CoverInfoRelative& info, QPixmap pixmap,
                                         bool fromCache) {
    Q_UNUSED(info);
    Q_UNUSED(fromCache);

    if (pRequestor == this && m_pLoadedTrack != nullptr &&
            m_pLoadedTrack->getCoverHash() == info.hash) {
        // qDebug() << "DlgCoverArtFullSize::slotCoverFound" << pRequestor << info
        //          << pixmap.size();
        m_pixmap = pixmap;
        // Scale down dialog if the pixmap is larger than the screen.
        // Use 90% of screen size instead of 100% to prevent an issue with
        // whitespace appearing on the side when resizing a window whose
        // borders touch the edges of the screen.
        QSize dialogSize = m_pixmap.size();
        const QSize availableScreenSpace =
            QApplication::desktop()->availableGeometry().size() * 0.9;
        if (dialogSize.height() > availableScreenSpace.height()) {
            dialogSize.scale(dialogSize.width(), availableScreenSpace.height(),
                             Qt::KeepAspectRatio);
        } else if (dialogSize.width() > availableScreenSpace.width()) {
            dialogSize.scale(availableScreenSpace.width(), dialogSize.height(),
                             Qt::KeepAspectRatio);
        }
        QPixmap resizedPixmap = m_pixmap.scaled(size(),
            Qt::KeepAspectRatio, Qt::SmoothTransformation);
        coverArt->setPixmap(resizedPixmap);
        // center the window
        setGeometry(QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                dialogSize,
                QApplication::desktop()->availableGeometry()));
    }
}

// slots to handle signals from the context menu
void DlgCoverArtFullSize::slotReloadCoverArt() {
    if (m_pLoadedTrack != nullptr) {
        auto coverInfo =
                CoverArtUtils::guessCoverInfo(*m_pLoadedTrack);
        slotCoverInfoSelected(coverInfo);
    }
}

void DlgCoverArtFullSize::slotCoverInfoSelected(const CoverInfoRelative& coverInfo) {
    // qDebug() << "DlgCoverArtFullSize::slotCoverInfoSelected" << coverInfo;
    if (m_pLoadedTrack != nullptr) {
        m_pLoadedTrack->setCoverInfo(coverInfo);
    }
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
    QPixmap resizedPixmap = m_pixmap.scaled(size(),
        Qt::KeepAspectRatio, Qt::SmoothTransformation);
    coverArt->setPixmap(resizedPixmap);
}

void DlgCoverArtFullSize::wheelEvent(QWheelEvent* event) {
    // Scale the image size
    int oldWidth = width();
    int oldHeight = height();
    int newWidth = oldWidth + (0.2 * event->delta());
    int newHeight = oldHeight + (0.2 * event->delta());
    QSize newSize = size();
    newSize.scale(newWidth, newHeight, Qt::KeepAspectRatio);

    // To keep the same part of the image under the cursor, shift the
    // origin (top left point) by the distance the point moves under the cursor.
    QPoint oldOrigin = geometry().topLeft();
    QPoint oldPointUnderCursor = event->pos();
    int newPointX = (double) oldPointUnderCursor.x() / oldWidth * newSize.width();
    int newPointY = (double) oldPointUnderCursor.y() / oldHeight * newSize.height();
    QPoint newOrigin = QPoint(
        oldOrigin.x() + (oldPointUnderCursor.x() - newPointX),
        oldOrigin.y() + (oldPointUnderCursor.y() - newPointY));

    // Calling resize() then move() causes flickering, so resize and move the window
    // simultaneously with setGeometry().
    setGeometry(QRect(newOrigin, newSize));

    event->accept();
}
