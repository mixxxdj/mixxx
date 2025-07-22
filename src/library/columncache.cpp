#include "library/columncache.h"

#include <QCoreApplication>

#include "library/dao/playlistdao.h"
#include "library/dao/trackschema.h"
#include "library/library_prefs.h"
#include "moc_columncache.cpp"
#include "util/db/dbconnection.h"

namespace {

const QString kSortInt = QStringLiteral("cast(%1 as integer)");
const QString kSortNoCase = QStringLiteral("lower(%1)");
const QString kSortNoCaseLex = mixxx::DbConnection::collateLexicographically(
        QStringLiteral("lower(%1)"));

struct ColumnProperties {
    const QString* pName;
    const char* pTrTitle;
    int defaultWidth;
};

constexpr int kDefaultColumnWidth = 50;

// MSVC does not support designated c99 initializers, but we use the support in
// the GCC build to verify that the array is complete and values are in the
// correct order.
#ifdef _MSC_VER
#define DI(index) // DESIGNATED_INITIALIZER
#else             // GCC (and other compilers that support designated initializers)
#define DI(index) [index] =
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc99-designator"
#endif
#endif

constexpr ColumnProperties kColumnPropertiesByEnum[] = {
        DI(ColumnCache::COLUMN_LIBRARYTABLE_ID){&LIBRARYTABLE_ID,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST){&LIBRARYTABLE_ARTIST,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Artist"),
                kDefaultColumnWidth * 4},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_TITLE){&LIBRARYTABLE_TITLE,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Title"),
                kDefaultColumnWidth * 4},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_ALBUM){&LIBRARYTABLE_ALBUM,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Album"),
                kDefaultColumnWidth * 4},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST){&LIBRARYTABLE_ALBUMARTIST,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Album Artist"),
                kDefaultColumnWidth * 4},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_YEAR){&LIBRARYTABLE_YEAR,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Year"),
                kDefaultColumnWidth},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_GENRE){&LIBRARYTABLE_GENRE,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Genre"),
                kDefaultColumnWidth * 4},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER){&LIBRARYTABLE_COMPOSER,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Composer"),
                kDefaultColumnWidth * 4},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_GROUPING){&LIBRARYTABLE_GROUPING,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Grouping"),
                kDefaultColumnWidth * 4},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER){&LIBRARYTABLE_TRACKNUMBER,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Track #"),
                kDefaultColumnWidth},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE){&LIBRARYTABLE_FILETYPE,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Type"),
                kDefaultColumnWidth},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_COMMENT){&LIBRARYTABLE_COMMENT,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Comment"),
                kDefaultColumnWidth * 6},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_DURATION){&LIBRARYTABLE_DURATION,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Duration"),
                kDefaultColumnWidth},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE){&LIBRARYTABLE_BITRATE,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Bitrate"),
                kDefaultColumnWidth},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_BPM){&LIBRARYTABLE_BPM,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "BPM"),
                kDefaultColumnWidth * 2},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN){&LIBRARYTABLE_REPLAYGAIN,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "ReplayGain"),
                kDefaultColumnWidth * 2},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_CUEPOINT){&LIBRARYTABLE_CUEPOINT,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_URL){&LIBRARYTABLE_URL,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_SAMPLERATE){&LIBRARYTABLE_SAMPLERATE,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Samplerate"),
                kDefaultColumnWidth},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_WAVESUMMARYHEX){&LIBRARYTABLE_WAVESUMMARYHEX,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Overview"),
                kDefaultColumnWidth * 8},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_CHANNELS){&LIBRARYTABLE_CHANNELS,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Channels"),
                kDefaultColumnWidth / 2},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_MIXXXDELETED){&LIBRARYTABLE_MIXXXDELETED,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED){&LIBRARYTABLE_DATETIMEADDED,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Date Added"),
                kDefaultColumnWidth * 3},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_HEADERPARSED){&LIBRARYTABLE_HEADERPARSED,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED){&LIBRARYTABLE_TIMESPLAYED,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Played"),
                kDefaultColumnWidth * 2},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED){&LIBRARYTABLE_PLAYED,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_RATING){&LIBRARYTABLE_RATING,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Rating"),
                kDefaultColumnWidth * 2},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_KEY){&LIBRARYTABLE_KEY,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Key"),
                kDefaultColumnWidth},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID){&LIBRARYTABLE_KEY_ID,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK){&LIBRARYTABLE_BPM_LOCK,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW){&LIBRARYTABLE_PREVIEW,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Preview"),
                kDefaultColumnWidth / 2},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_COLOR){&LIBRARYTABLE_COLOR,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Color"),
                kDefaultColumnWidth / 2},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_COVERART){&LIBRARYTABLE_COVERART,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Cover Art"),
                kDefaultColumnWidth / 2},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_SOURCE){&LIBRARYTABLE_COVERART_SOURCE,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_TYPE){&LIBRARYTABLE_COVERART_TYPE,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_LOCATION){&LIBRARYTABLE_COVERART_LOCATION,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_COLOR){&LIBRARYTABLE_COVERART_COLOR,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_DIGEST){&LIBRARYTABLE_COVERART_DIGEST,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_HASH){&LIBRARYTABLE_COVERART_HASH,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_LIBRARYTABLE_LAST_PLAYED_AT){&LIBRARYTABLE_LAST_PLAYED_AT,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Last Played"),
                kDefaultColumnWidth * 3},
        DI(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION){&TRACKLOCATIONSTABLE_LOCATION,
                QT_TRANSLATE_NOOP("BaseTrackTableModel", "Location"),
                kDefaultColumnWidth * 6},
        DI(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_FSDELETED){&TRACKLOCATIONSTABLE_FSDELETED,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_TRACKID){&PLAYLISTTRACKSTABLE_TRACKID,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION){&PLAYLISTTRACKSTABLE_POSITION,
                QT_TRANSLATE_NOOP("BaseSqlTableModel", "#"),
                kDefaultColumnWidth * 30 / 50},
        DI(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_PLAYLISTID){&PLAYLISTTRACKSTABLE_PLAYLISTID,
                nullptr,
                0},
        DI(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_DATETIMEADDED){
                &PLAYLISTTRACKSTABLE_DATETIMEADDED,
                QT_TRANSLATE_NOOP("BaseSqlTableModel", "Timestamp"),
                kDefaultColumnWidth * 80 / 50},
        DI(ColumnCache::COLUMN_REKORDBOX_ANALYZE_PATH){&REKORDBOX_ANALYZE_PATH, nullptr, 0}};
static_assert(std::size(kColumnPropertiesByEnum) == ColumnCache::NUM_COLUMNS);

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

} // namespace

ColumnCache::ColumnCache() {
    m_pKeyNotationCP = new ControlProxy(mixxx::library::prefs::kKeyNotationConfigKey, this);
    m_pKeyNotationCP->connectValueChanged(this, &ColumnCache::slotSetKeySortOrder);

    // ColumnCache is initialized before the preferences, so slotSetKeySortOrder is called
    // for again if DlgPrefKey sets the [Library]. key_notation CO to a value other than
    // KeyUtils::CUSTOM as Mixxx is starting.
}

ColumnCache::ColumnCache(QStringList columns)
        : ColumnCache::ColumnCache() {
    setColumns(std::move(columns));
}

void ColumnCache::setColumns(QStringList columns) {
    // This is called again when a new playlist/crate is selected
    // TODO: Split using code to remove this redundant initialization
    m_columnsByIndex = std::move(columns);

    m_columnIndexByName.clear();
    for (int i = 0; i < m_columnsByIndex.size(); ++i) {
        const QString& column = m_columnsByIndex[i];
        DEBUG_ASSERT(!m_columnIndexByName.contains(column));
        m_columnIndexByName[column] = i;
    }

    static_assert(std::size(kColumnPropertiesByEnum) ==
            sizeof(m_columnIndexByEnum) / sizeof(m_columnIndexByEnum[0]));
    for (std::size_t i = 0; i < std::size(kColumnPropertiesByEnum); ++i) {
        m_columnIndexByEnum[i] = fieldIndex(columnName(static_cast<Column>(i)));
    }

    m_columnSortByIndex.clear();
    // Add the columns that requires a special sort
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_ARTIST, kSortNoCaseLex);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_TITLE, kSortNoCaseLex);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_ALBUM, kSortNoCaseLex);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_ALBUMARTIST, kSortNoCaseLex);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_YEAR, kSortNoCase);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_GENRE, kSortNoCaseLex);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_COMPOSER, kSortNoCaseLex);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_GROUPING, kSortNoCaseLex);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_TRACKNUMBER, kSortInt);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_FILETYPE, kSortNoCase);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_COMMENT, kSortNoCaseLex);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_BITRATE, kSortInt);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_SAMPLERATE, kSortInt);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_TIMESPLAYED, kSortInt);

    insertColumnSortByEnum(COLUMN_TRACKLOCATIONSTABLE_LOCATION, kSortNoCase);

    slotSetKeySortOrder(m_pKeyNotationCP->get());
}

void ColumnCache::slotSetKeySortOrder(double notationValue) {
    const int keyColumnIndex = m_columnIndexByEnum[COLUMN_LIBRARYTABLE_KEY];
    if (keyColumnIndex < 0) {
        return;
    }

    // A custom COLLATE function was tested, but using CASE ... WHEN was found to be faster
    // see GitHub PR#649
    // https://github.com/mixxxdj/mixxx/pull/649#discussion_r34863809
    const auto notation =
            KeyUtils::keyNotationFromNumericValue(notationValue);
    // The placeholder %1 will be replaced by the "key" column. The actual
    // key code needed for sorting is stored in the column "key_id".
    DEBUG_ASSERT(LIBRARYTABLE_KEY_ID == LIBRARYTABLE_KEY + QStringLiteral("_id"));
    QString keySortSQL = QStringLiteral("CASE %1_id WHEN NULL THEN 0");
    for (int i = 0; i <= 24; ++i) {
        const auto sortOrder = KeyUtils::keyToCircleOfFifthsOrder(
                static_cast<mixxx::track::io::key::ChromaticKey>(i),
                notation);
        keySortSQL +=
                QStringLiteral(" WHEN ") +
                QString::number(i) +
                QStringLiteral(" THEN ") +
                QString::number(sortOrder);
    }
    keySortSQL.append(" END");

    // Replace the existing sort order
    m_columnSortByIndex[keyColumnIndex] = keySortSQL;
}

const QString& ColumnCache::columnName(Column column) const {
    DEBUG_ASSERT(static_cast<std::size_t>(column) < std::size(kColumnPropertiesByEnum));
    DEBUG_ASSERT(kColumnPropertiesByEnum[column].pName);
    return *kColumnPropertiesByEnum[column].pName;
}

QString ColumnCache::columnTitle(Column column) const {
    DEBUG_ASSERT(static_cast<std::size_t>(column) < std::size(kColumnPropertiesByEnum));
    return QCoreApplication::translate(
            "BaseTrackTableModel", kColumnPropertiesByEnum[column].pTrTitle);
}

int ColumnCache::columnDefaultWidth(Column column) const {
    DEBUG_ASSERT(static_cast<std::size_t>(column) < std::size(kColumnPropertiesByEnum));
    return kColumnPropertiesByEnum[column].defaultWidth;
}

// static
int ColumnCache::defaultColumnWidth() {
    return kDefaultColumnWidth;
}
