#include "library/tabledelegates/overviewdelegate.h"

#include <QMetaEnum>

#include "control/controlproxy.h"
#include "library/dao/trackdao.h"
#include "library/overviewcache.h"
#include "library/trackmodel.h"
#include "moc_overviewdelegate.cpp"
#include "util/logger.h"
#include "widget/wlibrary.h"

namespace {

const mixxx::Logger kLogger("OverviewDelegate");

inline TrackModel* asTrackModel(
        QTableView* pTableView) {
    auto* pTrackModel =
            dynamic_cast<TrackModel*>(pTableView->model());
    DEBUG_ASSERT(pTrackModel);
    return pTrackModel;
}

inline WLibrary* findLibraryWidgetParent(QWidget* pWidget) {
    while (pWidget) {
        WLibrary* pLibrary = qobject_cast<WLibrary*>(pWidget);
        if (pLibrary) {
            return pLibrary;
        }

        pWidget = pWidget->parentWidget();
    }

    return nullptr;
}

} // anonymous namespace

OverviewDelegate::OverviewDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView),
          m_pTrackModel(asTrackModel(pTableView)),
          m_pCache(OverviewCache::instance()),
          m_type(WOverview::Type::RGB) {
    VERIFY_OR_DEBUG_ASSERT(m_pCache) {
        kLogger.warning() << "Caching of overviews is not available";
        return;
    }

    WLibrary* pLibrary = findLibraryWidgetParent(pTableView);
    if (pLibrary) {
        m_signalColors = pLibrary->getOverviewSignalColors();
    }

    connect(m_pCache,
            &OverviewCache::overviewReady,
            this,
            &OverviewDelegate::slotOverviewReady);

    connect(m_pCache,
            &OverviewCache::overviewChanged,
            this,
            &OverviewDelegate::overviewChanged,
            Qt::DirectConnection); // signal-to-signal

    m_pTypeControl = make_parented<ControlProxy>(
            QStringLiteral("[Waveform]"),
            QStringLiteral("WaveformOverviewType"),
            this);
    m_pTypeControl->connectValueChanged(this, &OverviewDelegate::slotTypeControlChanged);
    slotTypeControlChanged(m_pTypeControl->get());
}

void OverviewDelegate::slotTypeControlChanged(double v) {
    // Assert that v is in enum range to prevent UB.
    DEBUG_ASSERT(v >= 0 && v < QMetaEnum::fromType<WOverview::Type>().keyCount());
    WOverview::Type type = static_cast<WOverview::Type>(static_cast<int>(v));
    if (type == m_type) {
        return;
    }

    m_type = type;
    // Instantly update visible overviews so we get a live preview
    // when changing the type in the preferences.
    m_pTableView->update();
}

/// Maybe request repaint via dataChanged() by BaseTrackTableModel
void OverviewDelegate::slotOverviewReady(const QObject* pRequester,
        const TrackId trackId,
        bool pixmapValid,
        const QSize resizedToSize) {
    Q_UNUSED(resizedToSize);
    // kLogger.info() << "slotOverviewReady()" << trackId << pixmap << resizedToSize;

    if (pRequester == this && pixmapValid) {
        emit overviewReady(trackId);
    }
}

void OverviewDelegate::paintItem(QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    paintItemBackground(painter, option, index);

    if (!m_pCache) {
        return;
    }

    const TrackId trackId(m_pTrackModel->getTrackId(index));
    const double scaleFactor = m_pTableView->devicePixelRatioF();
    QPixmap pixmap = m_pCache->requestOverview(m_type,
            m_signalColors,
            trackId,
            this,
            option.rect.size() * scaleFactor);
    if (!pixmap.isNull()) {
        // We have a cached pixmap, paint it
        pixmap.setDevicePixelRatio(scaleFactor);
        painter->drawPixmap(option.rect, pixmap);
    }
}
