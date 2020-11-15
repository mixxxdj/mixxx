#ifndef MIXXX_TRACKSCHEMA_H
#define MIXXX_TRACKSCHEMA_H

#include <QString>
#include <QStringList>

#define LIBRARY_TABLE "library"

const QString LIBRARYTABLE_ID = "id";
const QString LIBRARYTABLE_ARTIST = "artist";
const QString LIBRARYTABLE_TITLE = "title";
const QString LIBRARYTABLE_ALBUM = "album";
const QString LIBRARYTABLE_ALBUMARTIST = "album_artist";
const QString LIBRARYTABLE_YEAR = "year";
const QString LIBRARYTABLE_GENRE = "genre";
const QString LIBRARYTABLE_COMPOSER = "composer";
const QString LIBRARYTABLE_GROUPING = "grouping";
const QString LIBRARYTABLE_TRACKNUMBER = "tracknumber";
const QString LIBRARYTABLE_FILETYPE = "filetype";
const QString LIBRARYTABLE_LOCATION = "location";
const QString LIBRARYTABLE_COMMENT = "comment";
const QString LIBRARYTABLE_DURATION = "duration";
const QString LIBRARYTABLE_BITRATE = "bitrate";
const QString LIBRARYTABLE_BPM = "bpm";
const QString LIBRARYTABLE_REPLAYGAIN = "replaygain";
const QString LIBRARYTABLE_CUEPOINT = "cuepoint";
const QString LIBRARYTABLE_URL = "url";
const QString LIBRARYTABLE_SAMPLERATE = "samplerate";
const QString LIBRARYTABLE_WAVESUMMARYHEX = "wavesummaryhex";
const QString LIBRARYTABLE_CHANNELS = "channels";
const QString LIBRARYTABLE_MIXXXDELETED = "mixxx_deleted";
const QString LIBRARYTABLE_DATETIMEADDED = "datetime_added";
const QString LIBRARYTABLE_HEADERPARSED = "header_parsed";
const QString LIBRARYTABLE_TIMESPLAYED = "timesplayed";
const QString LIBRARYTABLE_PLAYED = "played";
const QString LIBRARYTABLE_RATING = "rating";
const QString LIBRARYTABLE_KEY = "key";
const QString LIBRARYTABLE_KEY_ID = "key_id";
const QString LIBRARYTABLE_BPM_LOCK = "bpm_lock";
const QString LIBRARYTABLE_PREVIEW = "preview";
const QString LIBRARYTABLE_COLOR = "color";
const QString LIBRARYTABLE_COVERART = "coverart";
const QString LIBRARYTABLE_COVERART_SOURCE = "coverart_source";
const QString LIBRARYTABLE_COVERART_TYPE = "coverart_type";
const QString LIBRARYTABLE_COVERART_LOCATION = "coverart_location";
const QString LIBRARYTABLE_COVERART_HASH = "coverart_hash";

const QString TRACKLOCATIONSTABLE_ID = "id";
const QString TRACKLOCATIONSTABLE_LOCATION = "location";
const QString TRACKLOCATIONSTABLE_FILENAME = "filename";
const QString TRACKLOCATIONSTABLE_DIRECTORY = "directory";
const QString TRACKLOCATIONSTABLE_FILESIZE = "filesize";
const QString TRACKLOCATIONSTABLE_FSDELETED = "fs_deleted";
const QString TRACKLOCATIONSTABLE_NEEDSVERIFICATION = "needs_verification";

const QString REKORDBOX_ANALYZE_PATH = "analyze_path";

// Simple static-only class for getting the ordered subset of columns used in the track headers.
class TrackSchema {
public:
    // GetColumns returns a QStringList of all the relevant columns that can be displayed to the
    // user.
    static QStringList GetColumnNames();

    // TableForColumn returns the name of the table that contains the named column.
    static QString TableForColumn(const QString& columnName);
};

#endif //MIXXX_TRACKSCHEMA_H
