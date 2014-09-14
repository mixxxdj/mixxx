#include <QDesktopWidget>

#include "dlgcoverartfullsize.h"

DlgCoverArtFullSize::DlgCoverArtFullSize() {
    setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
}

DlgCoverArtFullSize::~DlgCoverArtFullSize() {
}

void DlgCoverArtFullSize::init(CoverInfo info) {
    // As this dialog box will always be opened from a
    // cover widget (with a loaded cover), consequently the
    // target pixmap will already be in the QPixmapCache.
    // So, it just have to request the cached pixmap.
    QPixmap pixmap = CoverArtCache::instance()->requestPixmap(info,
                                                              QSize(0,0),
                                                              true, true);
    if (pixmap.isNull())  {
        return;
    }

    // If cover is bigger than Mixxx, it must be resized!
    // In this case, it need to do a small adjust to make
    // this dlg a bit smaller than the Mixxx window.
    QSize mixxxSize = QApplication::activeWindow()->size() / qreal(1.2);
    if (pixmap.height() > mixxxSize.height()
            || pixmap.width() > mixxxSize.width()) {
        pixmap.scaled(mixxxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    resize(pixmap.size());
    coverArt->setPixmap(pixmap);

    show();
    move(QApplication::desktop()->screenGeometry().center() - rect().center());
}
