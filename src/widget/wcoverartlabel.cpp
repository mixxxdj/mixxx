#include "widget/wcoverartlabel.h"

#include <QContextMenuEvent>

#include "library/coverartutils.h"
#include "library/dlgcoverartfullsize.h"
#include "moc_wcoverartlabel.cpp"
#include "widget/wcoverartmenu.h"

namespace {

// Device-independent size for the label
constexpr QSize kDefaultSize = QSize(100, 100);

// Size for the pixmap. Assumes frame width is 1px.
constexpr QSize kDefaultPixmapSize = kDefaultSize - QSize(2, 2);

inline QPixmap scaleCoverLabel(
        QLabel* pLabel,
        QPixmap pixmap,
        QSize size) {
    VERIFY_OR_DEBUG_ASSERT(size.isValid()) {
        size = kDefaultPixmapSize;
    }
    const auto devicePixelRatioF = pLabel->devicePixelRatioF();
    pixmap.setDevicePixelRatio(devicePixelRatioF);
    return pixmap.scaled(
            size * devicePixelRatioF,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
}

QPixmap createDefaultCover(QLabel* pLabel, QSize size) {
    auto defaultCover = QPixmap(CoverArtUtils::defaultCoverLocation());
    return scaleCoverLabel(pLabel, defaultCover, size);
}

} // anonymous namespace

WCoverArtLabel::WCoverArtLabel(QWidget* pParent, WCoverArtMenu* pCoverMenu)
        : QLabel(pParent),
          m_pCoverMenu(pCoverMenu),
          m_pDlgFullSize(make_parented<DlgCoverArtFullSize>(this, nullptr, pCoverMenu)),
          m_maxSize(kDefaultSize),
          m_pixmapSizeMax(kDefaultPixmapSize),
          m_defaultCover(createDefaultCover(this, m_pixmapSizeMax)) {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFrameShape(QFrame::Box);
    setAlignment(Qt::AlignCenter);
    setPixmapAndResize(m_defaultCover);
}

WCoverArtLabel::~WCoverArtLabel() = default;

void WCoverArtLabel::setCoverInfoAndPixmap(const CoverInfo& coverInfo,
        const QPixmap& px) {
    if (m_pCoverMenu != nullptr) {
        m_pCoverMenu->setCoverArt(coverInfo);
    }
    setPixmapAndResize(px);
}

void WCoverArtLabel::setPixmapAndResize(const QPixmap& px) {
    if (px.isNull()) {
        m_loadedCover = px;
        m_fullSizeCover = px;
        setPixmap(m_defaultCover);
    } else {
        m_loadedCover = scaleCoverLabel(this, px, m_pixmapSizeMax);
        m_fullSizeCover = px;
        setPixmap(m_loadedCover);
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    QSize newSize = pixmap().size() / devicePixelRatioF();
#else
    QSize newSize = pixmap()->size() / devicePixelRatioF();
#endif
    // add the frame so the entire pixmap is visible
    newSize += QSize(frameWidth() * 2, frameWidth() * 2);
    if (size() != newSize) {
        setFixedSize(newSize);
    }
}

void WCoverArtLabel::setMaxSize(const QSize newSize) {
    if (newSize == m_maxSize) {
        return;
    }

    m_maxSize = newSize;
    m_pixmapSizeMax = newSize - QSize(frameWidth() * 2, frameWidth() * 2);
    // Skip resizing the pixmap and label if the pixmap already fits.
    // Check if we got more space in one dimension and don't need it
    // for the other.
    const QSize pixmapSize = pixmap().size() / devicePixelRatioF();
    if (m_pixmapSizeMax == pixmapSize ||
            (m_pixmapSizeMax.height() == pixmapSize.height() &&
                    m_pixmapSizeMax.width() > pixmapSize.width()) ||
            (m_pixmapSizeMax.width() == pixmapSize.width() &&
                    m_pixmapSizeMax.height() > pixmapSize.height())) {
        return;
    }

    m_defaultCover = createDefaultCover(this, m_pixmapSizeMax);
    setPixmapAndResize(m_fullSizeCover);
}

void WCoverArtLabel::slotCoverMenu(const QPoint& pos) {
    if (m_pCoverMenu == nullptr) {
        return;
    }
    m_pCoverMenu->popup(mapToGlobal(pos));
}

void WCoverArtLabel::contextMenuEvent(QContextMenuEvent* event) {
    if (m_pCoverMenu == nullptr) {
        return;
    }
    event->accept();
    m_pCoverMenu->popup(event->globalPos());
}

void WCoverArtLabel::loadTrack(TrackPointer pTrack) {
    m_pLoadedTrack = pTrack;
}

void WCoverArtLabel::mousePressEvent(QMouseEvent* event) {
    if (m_pCoverMenu != nullptr && m_pCoverMenu->isVisible()) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (m_pDlgFullSize->isVisible()) {
            m_pDlgFullSize->close();
        } else {
            if (m_loadedCover.isNull()) {
                // Nothing to show
                return;
            } else if (!m_pLoadedTrack && !m_fullSizeCover.isNull()) {
                m_pDlgFullSize->initFetchedCoverArt(m_fullSizeCover);
            } else {
                // Regular track cover
                m_pDlgFullSize->init(m_pLoadedTrack);
            }
        }
    }
}
