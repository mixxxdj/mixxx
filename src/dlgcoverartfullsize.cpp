#include <QtDebug>
#include "dlgcoverartfullsize.h"
#include "library/coverartcache.h"

DlgCoverArtFullSize::DlgCoverArtFullSize() {
    setupUi(this);
    setWindowTitle(tr("Cover Art"));
    connect(CoverArtCache::instance(), SIGNAL(pixmapFound(int, QPixmap)),
            this, SLOT(slotPixmapFound(int, QPixmap)), Qt::DirectConnection);
}

DlgCoverArtFullSize::~DlgCoverArtFullSize() {
}

void DlgCoverArtFullSize::slotPixmapFound(int trackId, QPixmap pixmap) {
    Q_UNUSED(trackId);
    m_cover = pixmap;

    if (!QApplication::activeWindow()) {
        return;
    }

    // If cover is bigger than Mixxx, it must be resized!
    // In this case, it need to do a small adjust to make
    // this dlg a bit smaller than the Mixxx window.
    QSize mixxxSize = QApplication::activeWindow()->size() / qreal(1.2);
    if (m_cover.height() > mixxxSize.height()
            || m_cover.width() > mixxxSize.width()) {
        m_cover.scaled(mixxxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    if (isVisible()) {
        init();
    }
}

void DlgCoverArtFullSize::init() {
    if (m_cover.isNull())  {
        return;
    }
    resize(m_cover.size());
    coverArt->setPixmap(m_cover);
    show();
}
