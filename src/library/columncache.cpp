#include "library/columncache.h"

#include "library/dao/trackdao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/cratedao.h"

void ColumnCache::setColumns(const QStringList& columns) {
    m_columnIndexByName.clear();
    for (int i = 0; i < columns.size(); ++i) {
        m_columnIndexByName[columns[i]] = i;
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
    m_columnIndexByEnum[COLUMN_LIBRARYTABLE_LOCATION] = fieldIndex(LIBRARYTABLE_LOCATION);
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

    m_columnIndexByEnum[COLUMN_CRATETRACKSTABLE_TRACKID] = fieldIndex(CRATETRACKSTABLE_TRACKID);
    m_columnIndexByEnum[COLUMN_CRATETRACKSTABLE_CRATEID] = fieldIndex(CRATETRACKSTABLE_CRATEID);

    // Set up the reverse mapping, ignoring columns that don't get displayed.
    for (int i = 0; i < NUM_COLUMNS; ++i) {
        int index = m_columnIndexByEnum[i];
        if (index < 0) {
            continue;
        }
        m_columnByIndex[index] = static_cast<Column>(i);
    }
}
