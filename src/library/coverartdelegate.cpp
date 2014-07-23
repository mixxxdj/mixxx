#include <QPainter>

#include "library/coverartcache.h"
#include "library/coverartdelegate.h"
#include "library/trackmodel.h"
#include "library/dao/trackdao.h"

CoverArtDelegate::CoverArtDelegate(QObject *parent)
        : QStyledItemDelegate(parent),
          m_pTableView(NULL),
          m_bIsLocked(false) {
    // This assumes that the parent is wtracktableview
    connect(parent, SIGNAL(lockCoverArtDelegate(bool)),
            this, SLOT(slotLock(bool)));

    if (QTableView *tableView = qobject_cast<QTableView*>(parent)) {
        m_pTableView = tableView;
    }
}

CoverArtDelegate::~CoverArtDelegate() {
}

void CoverArtDelegate::slotLock(bool lock) {
    m_bIsLocked = lock;
}

void CoverArtDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
    painter->save();

    // Populate the correct colors based on the styling
    QStyleOptionViewItem newOption = option;
    initStyleOption(&newOption, index);

    // Set the palette appropriately based on whether the row is selected or
    // not. We also have to check if it is inactive or not and use the
    // appropriate ColorGroup.
    if (newOption.state & QStyle::State_Selected) {
        QPalette::ColorGroup colorGroup =
                newOption.state & QStyle::State_Active ?
                QPalette::Active : QPalette::Inactive;
        painter->fillRect(newOption.rect,
            newOption.palette.color(colorGroup, QPalette::Highlight));
        painter->setBrush(newOption.palette.color(
            colorGroup, QPalette::HighlightedText));
    } else {
        painter->fillRect(newOption.rect, newOption.palette.base());
    }

    TrackModel* trackModel = dynamic_cast<TrackModel*>(m_pTableView->model());
    if (!trackModel) {
        return;
    }

    QString defaultLoc = CoverArtCache::instance()->getDefaultCoverLocation();
    QString coverLocation = index.sibling(index.row(), trackModel->fieldIndex(
                            LIBRARYTABLE_COVERART_LOCATION)).data().toString();
    QString md5Hash = index.sibling(index.row(), trackModel->fieldIndex(
                        LIBRARYTABLE_COVERART_MD5)).data().toString();
    int trackId = trackModel->getTrackId(index);
    bool drawPixmap = coverLocation != defaultLoc;
    QPixmap pixmap;
    if (drawPixmap) {
        // If the CoverDelegate is locked, it must not try
        // to load (from coverLocation) and search covers.
        // It means that in this cases it will just draw
        // covers which are already in the pixmapcache.
        pixmap = CoverArtCache::instance()->requestPixmap(
                    trackId, coverLocation, md5Hash, !m_bIsLocked, false);
    }
    drawPixmap = !pixmap.isNull();

    if (drawPixmap) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
        int width = 100;
        if (option.rect.width() < width) {
            width = option.rect.width();
        }
        int height = (float) option.rect.height() * pixmap.width() / width;
        QRect target(option.rect.x(), option.rect.y(),
                     width, option.rect.height());
        QRect source(0, 0, pixmap.width(), height);
        painter->drawPixmap(target, pixmap, source);
    }

    painter->restore();
}
