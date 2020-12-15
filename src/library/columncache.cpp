#include "library/columncache.h"

#include "library/dao/playlistdao.h"
#include "library/dao/trackschema.h"
#include "moc_columncache.cpp"

ColumnCache::ColumnCache(const QStringList& columns) {
    m_pKeyNotationCP = new ControlProxy("[Library]", "key_notation", this);
    m_pKeyNotationCP->connectValueChanged(this, &ColumnCache::slotSetKeySortOrder);

    // ColumnCache is initialized before the preferences, so slotSetKeySortOrder is called
    // for again if DlgPrefKey sets the [Library]. key_notation CO to a value other than
    // KeyUtils::CUSTOM as Mixxx is starting.

    setColumns(columns);
}

void ColumnCache::setColumns(const QStringList& columns) {
    m_columnsByIndex.clear();
    m_columnsByIndex.append(columns);

    m_columnIndexByName.clear();
    for (int i = 0; i < columns.size(); ++i) {
        QString column = columns[i];
        m_columnIndexByName[column] = i;
    }

    for (int i = 0; i < NUM_COLUMNS; ++i) {
        m_columnIndexByEnum[i] = -1;
    }

    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_ID] = fieldIndex(LIBRARYTABLE_ID);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_ARTIST] = fieldIndex(LIBRARYTABLE_ARTIST);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_TITLE] = fieldIndex(LIBRARYTABLE_TITLE);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_ALBUM] = fieldIndex(LIBRARYTABLE_ALBUM);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_ALBUMARTIST] = fieldIndex(LIBRARYTABLE_ALBUMARTIST);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_YEAR] = fieldIndex(LIBRARYTABLE_YEAR);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_GENRE] = fieldIndex(LIBRARYTABLE_GENRE);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_COMPOSER] = fieldIndex(LIBRARYTABLE_COMPOSER);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_GROUPING] = fieldIndex(LIBRARYTABLE_GROUPING);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_TRACKNUMBER] = fieldIndex(LIBRARYTABLE_TRACKNUMBER);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_FILETYPE] = fieldIndex(LIBRARYTABLE_FILETYPE);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_NATIVELOCATION] = fieldIndex(LIBRARYTABLE_LOCATION);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_COMMENT] = fieldIndex(LIBRARYTABLE_COMMENT);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_DURATION] = fieldIndex(LIBRARYTABLE_DURATION);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_BITRATE] = fieldIndex(LIBRARYTABLE_BITRATE);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_BPM] = fieldIndex(LIBRARYTABLE_BPM);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_REPLAYGAIN] = fieldIndex(LIBRARYTABLE_REPLAYGAIN);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_CUEPOINT] = fieldIndex(LIBRARYTABLE_CUEPOINT);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_URL] = fieldIndex(LIBRARYTABLE_URL);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_SAMPLERATE] = fieldIndex(LIBRARYTABLE_SAMPLERATE);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_WAVESUMMARYHEX] = fieldIndex(LIBRARYTABLE_WAVESUMMARYHEX);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_CHANNELS] = fieldIndex(LIBRARYTABLE_CHANNELS);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_MIXXXDELETED] = fieldIndex(LIBRARYTABLE_MIXXXDELETED);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_DATETIMEADDED] = fieldIndex(LIBRARYTABLE_DATETIMEADDED);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_HEADERPARSED] = fieldIndex(LIBRARYTABLE_HEADERPARSED);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_TIMESPLAYED] = fieldIndex(LIBRARYTABLE_TIMESPLAYED);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_PLAYED] = fieldIndex(LIBRARYTABLE_PLAYED);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_RATING] = fieldIndex(LIBRARYTABLE_RATING);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_KEY] = fieldIndex(LIBRARYTABLE_KEY);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_KEY_ID] = fieldIndex(LIBRARYTABLE_KEY_ID);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_BPM_LOCK] = fieldIndex(LIBRARYTABLE_BPM_LOCK);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_PREVIEW] = fieldIndex(LIBRARYTABLE_PREVIEW);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_COLOR] = fieldIndex(LIBRARYTABLE_COLOR);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_COVERART] = fieldIndex(LIBRARYTABLE_COVERART);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_COVERART_SOURCE] = fieldIndex(LIBRARYTABLE_COVERART_SOURCE);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_COVERART_TYPE] = fieldIndex(LIBRARYTABLE_COVERART_TYPE);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_COVERART_LOCATION] = fieldIndex(LIBRARYTABLE_COVERART_LOCATION);
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_COVERART_HASH] = fieldIndex(LIBRARYTABLE_COVERART_HASH);

    m_columnIndexByEnum[COLUMN_TRACKLOCATIONSTABLE_FSDELETED] = fieldIndex(TRACKLOCATIONSTABLE_FSDELETED);

    m_columnIndexByEnum[COLUMN_PLAYLISTTRACKSTABLE_TRACKID] = fieldIndex(PLAYLISTTRACKSTABLE_TRACKID);
    m_columnIndexByEnum[COLUMN_PLAYLISTTRACKSTABLE_POSITION] = fieldIndex(PLAYLISTTRACKSTABLE_POSITION);
    m_columnIndexByEnum[COLUMN_PLAYLISTTRACKSTABLE_PLAYLISTID] = fieldIndex(PLAYLISTTRACKSTABLE_PLAYLISTID);
    m_columnIndexByEnum[COLUMN_PLAYLISTTRACKSTABLE_LOCATION] = fieldIndex(PLAYLISTTRACKSTABLE_LOCATION);
    m_columnIndexByEnum[COLUMN_PLAYLISTTRACKSTABLE_ARTIST] = fieldIndex(PLAYLISTTRACKSTABLE_ARTIST);
    m_columnIndexByEnum[COLUMN_PLAYLISTTRACKSTABLE_TITLE] = fieldIndex(PLAYLISTTRACKSTABLE_TITLE);
    m_columnIndexByEnum[COLUMN_PLAYLISTTRACKSTABLE_DATETIMEADDED] = fieldIndex(PLAYLISTTRACKSTABLE_DATETIMEADDED);

    m_columnIndexByEnum[COLUMN_REKORDBOX_ANALYZE_PATH] = fieldIndex(REKORDBOX_ANALYZE_PATH);

    const QString sortInt("cast(%1 as integer)");
    const QString sortNoCase("lower(%1)");

    m_columnSortByIndex.clear();
    // Add the columns that requires a special sort
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_ARTIST], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_TITLE], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_ALBUM], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_ALBUMARTIST], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_YEAR], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_GENRE], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_COMPOSER], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_GROUPING], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_TRACKNUMBER], sortInt);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_FILETYPE], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_NATIVELOCATION], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_COMMENT], sortNoCase);

    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_PLAYLISTTRACKSTABLE_LOCATION], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_PLAYLISTTRACKSTABLE_ARTIST], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_PLAYLISTTRACKSTABLE_TITLE], sortNoCase);

    slotSetKeySortOrder(m_pKeyNotationCP->get());
}

void ColumnCache::slotSetKeySortOrder(double notationValue) {
    if (m_columnIndexByEnum[COLUMN_LIBRARYTABLE_KEY] < 0) {
        return;
    }

    // A custom COLLATE function was tested, but using CASE ... WHEN was found to be faster
    // see GitHub PR#649
    // https://github.com/mixxxdj/mixxx/pull/649#discussion_r34863809
    KeyUtils::KeyNotation notation =
            KeyUtils::keyNotationFromNumericValue(notationValue);
    QString keySortSQL("CASE %1_id WHEN NULL THEN 0 ");
    for (int i = 0; i <= 24; ++i) {
        keySortSQL.append(QString("WHEN %1 THEN %2 ")
            .arg(QString::number(i),
                 QString::number(KeyUtils::keyToCircleOfFifthsOrder(
                                     static_cast<mixxx::track::io::key::ChromaticKey>(i),
                                     notation))));
    }
    keySortSQL.append("END");

    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_KEY], keySortSQL);
}
