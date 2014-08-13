#include <QtDebug>
#include "dlgcoverartfullsize.h"

DlgCoverArtFullSize::DlgCoverArtFullSize() {
    setupUi(this);
}

DlgCoverArtFullSize::~DlgCoverArtFullSize() {
}

void DlgCoverArtFullSize::init(QPixmap cover, QString title) {
    if (cover.isNull())  {
        return;
    }

    setWindowTitle(title);

    // If cover is bigger than Mixxx, it must be resized!
    // In this case, it need to do a small adjust to make
    // this dlg a bit smaller than the Mixxx window.
    QSize mixxxSize = QApplication::activeWindow()->size() / qreal(1.2);
    if (cover.height() > mixxxSize.height()
            || cover.width() > mixxxSize.width()) {
        cover.scaled(mixxxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    coverArt->setPixmap(cover);

    show();
}
