#include <QDesktopWidget>

#include "dlgcoverartfullsize.h"

DlgCoverArtFullSize::DlgCoverArtFullSize(QWidget* parent)
        : QDialog(parent) {
    setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
}

DlgCoverArtFullSize::~DlgCoverArtFullSize() {
}

void DlgCoverArtFullSize::init(CoverInfo info) {
    // this cannot be null
    QWidget* activeWindow = QApplication::activeWindow();
    if (!activeWindow) {
        return;
    }

    CoverArtCache* cache = CoverArtCache::instance();
    if (info.coverLocation.isEmpty()) {
        info.coverLocation = cache->trackInDBHash(info.trackId);
    }

    if (info.coverLocation == cache->getDefaultCoverLocation())  {
        return;
    }

    QPixmap pixmap;
    if (info.coverLocation == "ID3TAG") {
        pixmap.convertFromImage(
            cache->extractEmbeddedCover(info.trackLocation));
    } else {
        pixmap = QPixmap(info.coverLocation);
    }

    if (pixmap.isNull()) {
        return;
    }

    // If cover is bigger than Mixxx, it must be resized!
    // In this case, it need to do a small adjust to make
    // this dlg a bit smaller than the Mixxx window.
    QSize mixxxSize = activeWindow->size() / qreal(1.2);
    if (pixmap.height() > mixxxSize.height()
            || pixmap.width() > mixxxSize.width()) {
        pixmap = pixmap.scaled(
            mixxxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    resize(pixmap.size());
    coverArt->setPixmap(pixmap);

    show();
    move(QApplication::desktop()->screenGeometry().center() - rect().center());
    raise();
}
