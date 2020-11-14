#include "library/basetracktablemodel.h"

#include "library/bpmdelegate.h"
#include "library/colordelegate.h"
#include "library/coverartcache.h"
#include "library/coverartdelegate.h"
#include "library/dao/trackschema.h"
#include "library/locationdelegate.h"
#include "library/previewbuttondelegate.h"
#include "library/stardelegate.h"
#include "library/starrating.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/compatibility.h"
#include "util/datetime.h"
#include "util/logger.h"
#include "widget/wlibrary.h"
#include "widget/wtracktableview.h"

namespace {

const mixxx::Logger kLogger("BaseTrackTableModel");

constexpr double kRelativeHeightOfCoverartToolTip =
        0.165; // Height of the image for the cover art tooltip (Relative to the available screen size)

const QStringList kDefaultTableColumns = {
        LIBRARYTABLE_ALBUM,
        LIBRARYTABLE_ALBUMARTIST,
        LIBRARYTABLE_ARTIST,
        LIBRARYTABLE_BPM,
        LIBRARYTABLE_BPM_LOCK,
        LIBRARYTABLE_BITRATE,
        LIBRARYTABLE_CHANNELS,
        LIBRARYTABLE_COLOR,
        LIBRARYTABLE_COMMENT,
        LIBRARYTABLE_COMPOSER,
        LIBRARYTABLE_COVERART,
        LIBRARYTABLE_DATETIMEADDED,
        LIBRARYTABLE_DURATION,
        LIBRARYTABLE_FILETYPE,
        LIBRARYTABLE_GENRE,
        LIBRARYTABLE_GROUPING,
        LIBRARYTABLE_KEY,
        LIBRARYTABLE_LOCATION,
        LIBRARYTABLE_PLAYED,
        LIBRARYTABLE_PREVIEW,
        LIBRARYTABLE_RATING,
        LIBRARYTABLE_REPLAYGAIN,
        LIBRARYTABLE_SAMPLERATE,
        LIBRARYTABLE_TIMESPLAYED,
        LIBRARYTABLE_TITLE,
        LIBRARYTABLE_TRACKNUMBER,
        LIBRARYTABLE_YEAR,
};

inline QSqlDatabase cloneDatabase(
        const QSqlDatabase& prototype) {
    const auto connectionName =
            uuidToStringWithoutBraces(QUuid::createUuid());
    auto cloned = QSqlDatabase::cloneDatabase(
            prototype,
            connectionName);
    DEBUG_ASSERT(cloned.isValid());
    if (prototype.isOpen() && !cloned.open()) {
        kLogger.warning()
                << "Failed to open cloned database connection"
                << cloned
                << cloned.lastError();
    }
    return cloned;
}

QSqlDatabase cloneDatabase(
        TrackCollectionManager* pTrackCollectionManager) {
    VERIFY_OR_DEBUG_ASSERT(pTrackCollectionManager &&
            pTrackCollectionManager->internalCollection()) {
        return QSqlDatabase();
    }
    const auto connectionName =
            uuidToStringWithoutBraces(QUuid::createUuid());
    return cloneDatabase(
            pTrackCollectionManager->internalCollection()->database());
}

} // anonymous namespace

//static
QStringList BaseTrackTableModel::defaultTableColumns() {
    return kDefaultTableColumns;
}

BaseTrackTableModel::BaseTrackTableModel(
        QObject* parent,
        TrackCollectionManager* pTrackCollectionManager,
        const char* settingsNamespace)
        : QAbstractTableModel(parent),
          TrackModel(cloneDatabase(pTrackCollectionManager), settingsNamespace),
          m_pTrackCollectionManager(pTrackCollectionManager),
          m_previewDeckGroup(PlayerManager::groupForPreviewDeck(0)),
          m_backgroundColorOpacity(WLibrary::kDefaultTrackTableBackgroundColorOpacity) {
    connect(&pTrackCollectionManager->internalCollection()->getTrackDAO(),
            &TrackDAO::forceModelUpdate,
            this,
            &BaseTrackTableModel::slotRefreshAllRows);
    connect(&PlayerInfo::instance(),
            &PlayerInfo::trackLoaded,
            this,
            &BaseTrackTableModel::slotTrackLoaded);
}

void BaseTrackTableModel::initTableColumnsAndHeaderProperties(
        const QStringList& tableColumns) {
    m_columnCache.setColumns(tableColumns);
    if (m_columnHeaders.size() < tableColumns.size()) {
        m_columnHeaders.resize(tableColumns.size());
    }
    initHeaderProperties();
}

void BaseTrackTableModel::initHeaderProperties() {
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_ALBUM,
            tr("Album"),
            defaultColumnWidth() * 4);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST,
            tr("Album Artist"),
            defaultColumnWidth() * 4);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_ARTIST,
            tr("Artist"),
            defaultColumnWidth() * 4);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_BITRATE,
            tr("Bitrate"),
            defaultColumnWidth());
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_BPM,
            tr("BPM"),
            defaultColumnWidth() * 2);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_CHANNELS,
            tr("Channels"),
            defaultColumnWidth() / 2);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_COLOR,
            tr("Color"),
            defaultColumnWidth() / 2);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_COMMENT,
            tr("Comment"),
            defaultColumnWidth() * 6);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER,
            tr("Composer"),
            defaultColumnWidth() * 4);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_COVERART,
            tr("Cover Art"),
            defaultColumnWidth() / 2);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED,
            tr("Date Added"),
            defaultColumnWidth() * 3);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_DURATION,
            tr("Duration"),
            defaultColumnWidth());
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE,
            tr("Type"),
            defaultColumnWidth());
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_GENRE,
            tr("Genre"),
            defaultColumnWidth() * 4);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_GROUPING,
            tr("Grouping"),
            defaultColumnWidth() * 4);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_KEY,
            tr("Key"),
            defaultColumnWidth());
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_NATIVELOCATION,
            tr("Location"),
            defaultColumnWidth() * 6);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW,
            tr("Preview"),
            defaultColumnWidth() / 2);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_RATING,
            tr("Rating"),
            defaultColumnWidth() * 2);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN,
            tr("ReplayGain"),
            defaultColumnWidth() * 2);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_SAMPLERATE,
            tr("Samplerate"),
            defaultColumnWidth());
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED,
            tr("Played"),
            defaultColumnWidth() * 2);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_TITLE,
            tr("Title"),
            defaultColumnWidth() * 4);
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER,
            tr("Track #"),
            defaultColumnWidth());
    setHeaderProperties(
            ColumnCache::COLUMN_LIBRARYTABLE_YEAR,
            tr("Year"),
            defaultColumnWidth());
}

void BaseTrackTableModel::setHeaderProperties(
        ColumnCache::Column column,
        QString title,
        int defaultWidth) {
    int section = fieldIndex(column);
    if (section < 0) {
        kLogger.debug()
                << "Skipping header properties for unsupported column"
                << column
                << title;
        return;
    }
    if (section >= m_columnHeaders.size()) {
        m_columnHeaders.resize(section + 1);
    }
    m_columnHeaders[section].column = column;
    setHeaderData(
            section,
            Qt::Horizontal,
            m_columnCache.columnName(column),
            TrackModel::kHeaderNameRole);
    setHeaderData(
            section,
            Qt::Horizontal,
            title,
            Qt::DisplayRole);
    setHeaderData(
            section,
            Qt::Horizontal,
            defaultWidth,
            TrackModel::kHeaderWidthRole);
}

bool BaseTrackTableModel::setHeaderData(
        int section,
        Qt::Orientation orientation,
        const QVariant& value,
        int role) {
    VERIFY_OR_DEBUG_ASSERT(section >= 0) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(section < m_columnHeaders.size()) {
        return false;
    }
    if (orientation != Qt::Horizontal) {
        // We only care about horizontal headers.
        return false;
    }
    m_columnHeaders[section].header[role] = value;
    emit headerDataChanged(orientation, section, section);
    return true;
}

QVariant BaseTrackTableModel::headerData(
        int section,
        Qt::Orientation orientation,
        int role) const {
    if (orientation == Qt::Horizontal) {
        switch (role) {
        case Qt::DisplayRole: {
            QVariant headerValue =
                    m_columnHeaders.value(section).header.value(role);
            if (!headerValue.isValid()) {
                // Try EditRole if DisplayRole wasn't present
                headerValue = m_columnHeaders.value(section).header.value(Qt::EditRole);
            }
            if (headerValue.isValid()) {
                return headerValue;
            } else {
                return QVariant(section).toString();
            }
        }
        case TrackModel::kHeaderWidthRole: {
            QVariant widthValue = m_columnHeaders.value(section).header.value(role);
            if (widthValue.isValid()) {
                return widthValue;
            } else {
                return defaultColumnWidth();
            }
        }
        case TrackModel::kHeaderNameRole: {
            return m_columnHeaders.value(section).header.value(role);
        }
        case Qt::ToolTipRole: {
            QVariant tooltip = m_columnHeaders.value(section).header.value(role);
            if (tooltip.isValid()) {
                return tooltip;
            }
            break;
        }
        default:
            break;
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

int BaseTrackTableModel::countValidColumnHeaders() const {
    int count = 0;
    for (const auto& columnHeader : m_columnHeaders) {
        if (columnHeader.column !=
                ColumnCache::COLUMN_LIBRARYTABLE_INVALID) {
            ++count;
        }
    }
    return count;
}

int BaseTrackTableModel::columnCount(const QModelIndex& parent) const {
    VERIFY_OR_DEBUG_ASSERT(!parent.isValid()) {
        return 0;
    }
    return countValidColumnHeaders();
}

bool BaseTrackTableModel::isColumnHiddenByDefault(
        int column) {
    return column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_CHANNELS) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GROUPING) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_NATIVELOCATION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_SAMPLERATE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_YEAR);
}

QAbstractItemDelegate* BaseTrackTableModel::delegateForColumn(
        const int index, QObject* pParent) {
    auto* pTableView = qobject_cast<WTrackTableView*>(pParent);
    VERIFY_OR_DEBUG_ASSERT(pTableView) {
        return nullptr;
    }
    m_backgroundColorOpacity = pTableView->getBackgroundColorOpacity();
    if (index == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING)) {
        return new StarDelegate(pTableView);
    } else if (index == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM)) {
        return new BPMDelegate(pTableView);
    } else if (PlayerManager::numPreviewDecks() > 0 &&
            index == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)) {
        return new PreviewButtonDelegate(pTableView, index);
    } else if (index == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_NATIVELOCATION)) {
        return new LocationDelegate(pTableView);
    } else if (index == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COLOR)) {
        return new ColorDelegate(pTableView);
    } else if (index == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART)) {
        auto* pCoverArtDelegate =
                new CoverArtDelegate(pTableView);
        // WLibraryTableView -> CoverArtDelegate
        connect(pTableView,
                &WLibraryTableView::onlyCachedCoverArt,
                pCoverArtDelegate,
                &CoverArtDelegate::slotInhibitLazyLoading);
        // CoverArtDelegate -> BaseTrackTableModel
        connect(pCoverArtDelegate,
                &CoverArtDelegate::rowsChanged,
                this,
                &BaseTrackTableModel::slotRefreshCoverRows);
        return pCoverArtDelegate;
    }
    return nullptr;
}

QVariant BaseTrackTableModel::data(
        const QModelIndex& index,
        int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::BackgroundRole) {
        QModelIndex colorIndex = index.sibling(
                index.row(),
                fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COLOR));
        if (!colorIndex.isValid()) {
            return QVariant();
        }
        const auto trackColor =
                mixxx::RgbColor::fromQVariant(
                        rawValue(colorIndex));
        if (!trackColor) {
            return QVariant();
        }
        auto bgColor = mixxx::RgbColor::toQColor(trackColor);
        DEBUG_ASSERT(bgColor.isValid());
        DEBUG_ASSERT(m_backgroundColorOpacity >= 0.0);
        DEBUG_ASSERT(m_backgroundColorOpacity <= 1.0);
        bgColor.setAlphaF(m_backgroundColorOpacity);
        return QBrush(bgColor);
    }

    // Only retrieve a value for supported roles
    if (role != Qt::DisplayRole &&
            role != Qt::EditRole &&
            role != Qt::CheckStateRole &&
            role != Qt::ToolTipRole) {
        return QVariant();
    }

    return roleValue(index, rawValue(index), role);
}

bool BaseTrackTableModel::setData(
        const QModelIndex& index,
        const QVariant& value,
        int role) {
    const int column = index.column();

    // Override sets to TIMESPLAYED and redirect them to PLAYED
    if (role == Qt::CheckStateRole) {
        const auto val = value.toInt() > 0;
        if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED)) {
            QModelIndex playedIndex = index.sibling(index.row(), fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED));
            return setData(playedIndex, val, Qt::EditRole);
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM)) {
            QModelIndex bpmLockindex = index.sibling(index.row(), fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK));
            return setData(bpmLockindex, val, Qt::EditRole);
        }
        return false;
    }

    TrackPointer pTrack = getTrack(index);
    if (!pTrack) {
        return false;
    }

    // Do not save the track here. Changing the track dirties it and the caching
    // system will automatically save the track once it is unloaded from
    // memory. rryan 10/2010
    return setTrackValueForColumn(pTrack, column, value, role);
}

QVariant BaseTrackTableModel::composeCoverArtToolTipHtml(
        const QModelIndex& index) const {
    // Determine height of the cover art image depending on the screen size
    const QScreen* primaryScreen = getPrimaryScreen();
    if (!primaryScreen) {
        DEBUG_ASSERT(!"Primary screen not found!");
        return QVariant();
    }
    unsigned int absoluteHeightOfCoverartToolTip = static_cast<int>(
            primaryScreen->availableGeometry().height() *
            kRelativeHeightOfCoverartToolTip);
    // Get image from cover art cache
    CoverArtCache* pCache = CoverArtCache::instance();
    QPixmap pixmap = QPixmap(absoluteHeightOfCoverartToolTip,
            absoluteHeightOfCoverartToolTip); // Height also used as default for the width, in assumption that covers are squares
    pixmap = pCache->tryLoadCover(this,
            getCoverInfo(index),
            absoluteHeightOfCoverartToolTip,
            CoverArtCache::Loading::NoSignal);
    if (pixmap.isNull()) {
        // Cache miss -> Don't show a tooltip
        return QVariant();
    }
    QByteArray data;
    QBuffer buffer(&data);
    pixmap.save(&buffer, "BMP"); // Binary bitmap format, without compression effort
    QString html = QString(
            "<img src='data:image/bmp;base64, %0'>")
                           .arg(QString::fromLatin1(data.toBase64()));
    return html;
}

QVariant BaseTrackTableModel::roleValue(
        const QModelIndex& index,
        QVariant&& rawValue,
        int role) const {
    const int column = index.column();
    // Format the value based on whether we are in a tooltip,
    // display, or edit role
    switch (role) {
    case Qt::ToolTipRole:
        if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COLOR)) {
            return mixxx::RgbColor::toQString(mixxx::RgbColor::fromQVariant(rawValue));
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART)) {
            return composeCoverArtToolTipHtml(index);
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)) {
            return QVariant();
        }
        M_FALLTHROUGH_INTENDED;
    case Qt::DisplayRole:
        if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DURATION)) {
            bool ok;
            const auto duration = rawValue.toDouble(&ok);
            if (ok && duration >= 0) {
                return mixxx::Duration::formatTime(
                        duration,
                        mixxx::Duration::Precision::SECONDS);
            } else {
                return QVariant();
            }
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING)) {
            VERIFY_OR_DEBUG_ASSERT(rawValue.canConvert(QMetaType::Int)) {
                return QVariant();
            }
            return QVariant::fromValue(StarRating(rawValue.toInt()));
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED)) {
            return rawValue.toBool();
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED)) {
            VERIFY_OR_DEBUG_ASSERT(rawValue.canConvert(QMetaType::Int)) {
                return QVariant();
            }
            return QString("(%1)").arg(rawValue.toInt());
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED) ||
                column == fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_DATETIMEADDED)) {
            return mixxx::localDateTimeFromUtc(mixxx::convertVariantToDateTime(rawValue));
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM)) {
            bool ok;
            const auto bpmValue = rawValue.toDouble(&ok);
            if (ok && bpmValue > 0.0) {
                return QString("%1").arg(bpmValue, 0, 'f', 1);
            } else {
                return QChar('-');
            }
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK)) {
            return rawValue.toBool();
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_YEAR)) {
            return mixxx::TrackMetadata::formatCalendarYear(rawValue.toString());
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER)) {
            const auto trackNumber = rawValue.toInt(0);
            if (trackNumber > 0) {
                return std::move(rawValue);
            } else {
                // clear invalid values
                return QVariant();
            }
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE)) {
            int bitrateValue = rawValue.toInt(0);
            if (bitrateValue > 0) {
                return std::move(rawValue);
            } else {
                // clear invalid values
                return QVariant();
            }
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY)) {
            // If we know the semantic key via the LIBRARYTABLE_KEY_ID
            // column (as opposed to the string representation of the key
            // currently stored in the DB) then lookup the key and render it
            // using the user's selected notation.
            int keyIdColumn = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID);
            if (keyIdColumn == -1) {
                // Otherwise, just use the column value
                return std::move(rawValue);
            }
            mixxx::track::io::key::ChromaticKey key =
                    KeyUtils::keyFromNumericValue(
                            index.sibling(index.row(), keyIdColumn).data().toInt());
            if (key == mixxx::track::io::key::INVALID) {
                // clear invalid values
                return QVariant();
            }
            // Render this key with the user-provided notation.
            return KeyUtils::keyToString(key);
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN)) {
            bool ok;
            const auto gainValue = rawValue.toDouble(&ok);
            return ok ? mixxx::ReplayGain::ratioToString(gainValue) : QString();
        }
        // Otherwise, just use the column value
        break;
    case Qt::EditRole:
        if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM)) {
            bool ok;
            const auto bpmValue = rawValue.toDouble(&ok);
            return ok ? bpmValue : 0.0;
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED)) {
            return index.sibling(
                                 index.row(), fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED))
                            .data()
                            .toBool();
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING)) {
            VERIFY_OR_DEBUG_ASSERT(rawValue.canConvert(QMetaType::Int)) {
                return QVariant();
            }
            return QVariant::fromValue(StarRating(rawValue.toInt()));
        }
        // Otherwise, just use the column value
        break;
    case Qt::CheckStateRole:
        if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED)) {
            bool played = index.sibling(
                                       index.row(), fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED))
                                  .data()
                                  .toBool();
            return played ? Qt::Checked : Qt::Unchecked;
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM)) {
            bool locked = index.sibling(
                                       index.row(), fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK))
                                  .data()
                                  .toBool();
            return locked ? Qt::Checked : Qt::Unchecked;
        }
        // No check state supported
        return QVariant();
    default:
        DEBUG_ASSERT(!"unexpected role");
        break;
    }
    return std::move(rawValue);
}

bool BaseTrackTableModel::isBpmLocked(
        const QModelIndex& index) const {
    const auto bpmLockIndex =
            index.sibling(
                    index.row(),
                    fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK));
    return bpmLockIndex.data().toBool();
}

Qt::ItemFlags BaseTrackTableModel::defaultItemFlags(
        const QModelIndex& index) const {
    if (index.isValid()) {
        return QAbstractItemModel::flags(index) |
                // Enable dragging songs from this data model to elsewhere
                // like the waveform widget to load a track into a Player
                Qt::ItemIsDragEnabled;
    } else {
        return Qt::ItemIsEnabled;
    }
}

Qt::ItemFlags BaseTrackTableModel::readOnlyFlags(
        const QModelIndex& index) const {
    return defaultItemFlags(index);
}

Qt::ItemFlags BaseTrackTableModel::readWriteFlags(
        const QModelIndex& index) const {
    if (!index.isValid()) {
        return defaultItemFlags(index);
    }

    const int column = index.column();
    if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_CHANNELS) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COLOR) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DURATION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_NATIVELOCATION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_SAMPLERATE)) {
        return readOnlyFlags(index);
    }

    Qt::ItemFlags itemFlags = defaultItemFlags(index);
    if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK)) {
        // Checkable cells
        itemFlags |= Qt::ItemIsUserCheckable;
    } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM)) {
        // Always allow checking of the BPM-locked indicator
        itemFlags |= Qt::ItemIsUserCheckable;
        // Allow editing of BPM only if not locked
        if (!isBpmLocked(index)) {
            itemFlags |= Qt::ItemIsEditable;
        }
    } else {
        // Cells are editable by default
        itemFlags |= Qt::ItemIsEditable;
    }
    return itemFlags;
}

Qt::ItemFlags BaseTrackTableModel::flags(
        const QModelIndex& index) const {
    return readWriteFlags(index);
}

QList<QUrl> BaseTrackTableModel::collectUrls(
        const QModelIndexList& indexes) const {
    QList<QUrl> urls;
    urls.reserve(indexes.size());
    // The list of indexes we're given contains separates indexes for each
    // column, so even if only one row is selected, we'll have columnCount()
    // indices.  We need to only count each row once:
    QSet<int> visitedRows;
    for (const auto& index : indexes) {
        if (visitedRows.contains(index.row())) {
            continue;
        }
        visitedRows.insert(index.row());
        QUrl url = TrackFile(getTrackLocation(index)).toUrl();
        if (url.isValid()) {
            urls.append(url);
        }
    }
    return urls;
}

QMimeData* BaseTrackTableModel::mimeData(
        const QModelIndexList& indexes) const {
    const auto urls = collectUrls(indexes);
    if (urls.isEmpty()) {
        return nullptr;
    } else {
        QMimeData* mimeData = new QMimeData();
        mimeData->setUrls(urls);
        return mimeData;
    }
}

void BaseTrackTableModel::slotTrackLoaded(
        QString group,
        TrackPointer pTrack) {
    if (group == m_previewDeckGroup) {
        // If there was a previously loaded track, refresh its rows so the
        // preview state will update.
        if (m_previewDeckTrackId.isValid()) {
            const int numColumns = columnCount();
            const auto rows = getTrackRows(m_previewDeckTrackId);
            m_previewDeckTrackId = TrackId(); // invalidate
            for (int row : rows) {
                QModelIndex topLeft = index(row, 0);
                QModelIndex bottomRight = index(row, numColumns);
                emit dataChanged(topLeft, bottomRight);
            }
        }
        m_previewDeckTrackId = doGetTrackId(pTrack);
    }
}

void BaseTrackTableModel::slotRefreshCoverRows(
        QList<int> rows) {
    if (rows.isEmpty()) {
        return;
    }
    const int column = fieldIndex(LIBRARYTABLE_COVERART);
    VERIFY_OR_DEBUG_ASSERT(column >= 0) {
        return;
    }
    emitDataChangedForMultipleRowsInColumn(rows, column);
}

void BaseTrackTableModel::slotRefreshAllRows() {
    select();
}

void BaseTrackTableModel::emitDataChangedForMultipleRowsInColumn(
        const QList<int>& rows,
        int column,
        const QVector<int>& roles) {
    DEBUG_ASSERT(column >= 0);
    DEBUG_ASSERT(column < columnCount());
    int beginRow = -1;
    int endRow = -1;
    for (const int row : rows) {
        DEBUG_ASSERT(row >= rows.first());
        DEBUG_ASSERT(row <= rows.last());
        DEBUG_ASSERT(row >= 0);
        if (row >= rowCount()) {
            // The number of rows might have changed since the signal
            // has been emitted. This case seems to occur after switching
            // to a different view with less rows.
            continue;
        }
        if (beginRow < 0) {
            // Start the first stride
            DEBUG_ASSERT(beginRow == endRow);
            DEBUG_ASSERT(row == rows.first());
            beginRow = row;
            endRow = row + 1;
        } else if (row == endRow) {
            // Continue the current stride
            ++endRow;
        } else {
            // Finish the current stride...
            DEBUG_ASSERT(beginRow >= rows.first());
            DEBUG_ASSERT(beginRow < endRow);
            DEBUG_ASSERT(endRow - 1 <= rows.last());
            QModelIndex topLeft = index(beginRow, column);
            QModelIndex bottomRight = index(endRow - 1, column);
            emit dataChanged(topLeft, bottomRight, roles);
            // ...before starting the next stride
            // Rows are expected to be sorted in ascending order
            // without duplicates!
            DEBUG_ASSERT(row >= endRow);
            beginRow = row;
            endRow = row + 1;
        }
    }
    if (beginRow < endRow) {
        // Finish the final stride
        DEBUG_ASSERT(beginRow >= rows.first());
        DEBUG_ASSERT(endRow - 1 <= rows.last());
        QModelIndex topLeft = index(beginRow, column);
        QModelIndex bottomRight = index(endRow - 1, column);
        emit dataChanged(topLeft, bottomRight, roles);
    }
}

TrackPointer BaseTrackTableModel::getTrackByRef(
        const TrackRef& trackRef) const {
    return m_pTrackCollectionManager->internalCollection()->getTrackByRef(trackRef);
}

TrackId BaseTrackTableModel::doGetTrackId(
        const TrackPointer& pTrack) const {
    return pTrack ? pTrack->getId() : TrackId();
}
