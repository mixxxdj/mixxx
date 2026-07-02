#pragma once

#include <QString>

#define LIBRARY_TABLE "library"
#define TRACKLOCATIONS_TABLE "track_locations"

#define PLAYLIST_TABLE "Playlists"
#define PLAYLIST_TRACKS_TABLE "PlaylistTracks"

#define AUTODJ_TABLE "Auto DJ"

const QString LIBRARYTABLE_ID = QStringLiteral("id");
const QString LIBRARYTABLE_ARTIST = QStringLiteral("artist");
const QString LIBRARYTABLE_TITLE = QStringLiteral("title");
const QString LIBRARYTABLE_ALBUM = QStringLiteral("album");
const QString LIBRARYTABLE_ALBUMARTIST = QStringLiteral("album_artist");
const QString LIBRARYTABLE_YEAR = QStringLiteral("year");
const QString LIBRARYTABLE_GENRE = QStringLiteral("genre");
const QString LIBRARYTABLE_COMPOSER = QStringLiteral("composer");
const QString LIBRARYTABLE_GROUPING = QStringLiteral("grouping");
const QString LIBRARYTABLE_TRACKNUMBER = QStringLiteral("tracknumber");
const QString LIBRARYTABLE_FILETYPE = QStringLiteral("filetype");
const QString LIBRARYTABLE_LOCATION = QStringLiteral("location");
const QString LIBRARYTABLE_COMMENT = QStringLiteral("comment");
const QString LIBRARYTABLE_DURATION = QStringLiteral("duration");
const QString LIBRARYTABLE_BITRATE = QStringLiteral("bitrate");
const QString LIBRARYTABLE_BPM = QStringLiteral("bpm");
const QString LIBRARYTABLE_REPLAYGAIN = QStringLiteral("replaygain");
const QString LIBRARYTABLE_CUEPOINT = QStringLiteral("cuepoint");
const QString LIBRARYTABLE_URL = QStringLiteral("url");
const QString LIBRARYTABLE_SAMPLERATE = QStringLiteral("samplerate");
const QString LIBRARYTABLE_WAVESUMMARYHEX = QStringLiteral("wavesummaryhex");
const QString LIBRARYTABLE_CHANNELS = QStringLiteral("channels");
const QString LIBRARYTABLE_MIXXXDELETED = QStringLiteral("mixxx_deleted");
const QString LIBRARYTABLE_DATETIMEADDED = QStringLiteral("datetime_added");
const QString LIBRARYTABLE_HEADERPARSED = QStringLiteral("header_parsed");
const QString LIBRARYTABLE_TIMESPLAYED = QStringLiteral("timesplayed");
const QString LIBRARYTABLE_LAST_PLAYED_AT = QStringLiteral("last_played_at");
const QString LIBRARYTABLE_PLAYED = QStringLiteral("played");
const QString LIBRARYTABLE_RATING = QStringLiteral("rating");
const QString LIBRARYTABLE_KEY = QStringLiteral("key");
const QString LIBRARYTABLE_KEY_ID = QStringLiteral("key_id");
const QString LIBRARYTABLE_TUNING_FREQUENCY = QStringLiteral("tuning_frequency_hz");
const QString LIBRARYTABLE_BPM_LOCK = QStringLiteral("bpm_lock");
const QString LIBRARYTABLE_BEATS_VERSION = QStringLiteral("beats_version");
const QString LIBRARYTABLE_PREVIEW = QStringLiteral("preview");
const QString LIBRARYTABLE_COLOR = QStringLiteral("color");
const QString LIBRARYTABLE_COVERART = QStringLiteral("coverart");
const QString LIBRARYTABLE_COVERART_SOURCE = QStringLiteral("coverart_source");
const QString LIBRARYTABLE_COVERART_TYPE = QStringLiteral("coverart_type");
const QString LIBRARYTABLE_COVERART_LOCATION = QStringLiteral("coverart_location");
const QString LIBRARYTABLE_COVERART_COLOR = QStringLiteral("coverart_color");
const QString LIBRARYTABLE_COVERART_DIGEST = QStringLiteral("coverart_digest");
const QString LIBRARYTABLE_COVERART_HASH = QStringLiteral("coverart_hash");
const QString LIBRARYTABLE_CRATE = QStringLiteral("crate");

const QString TRACKLOCATIONSTABLE_ID = QStringLiteral("id");
const QString TRACKLOCATIONSTABLE_LOCATION = QStringLiteral("location");
const QString TRACKLOCATIONSTABLE_FILENAME = QStringLiteral("filename");
const QString TRACKLOCATIONSTABLE_DIRECTORY = QStringLiteral("directory");
const QString TRACKLOCATIONSTABLE_FILESIZE = QStringLiteral("filesize");
const QString TRACKLOCATIONSTABLE_FSDELETED = QStringLiteral("fs_deleted");
const QString TRACKLOCATIONSTABLE_NEEDSVERIFICATION = QStringLiteral("needs_verification");

const QString TRACK_ID = QStringLiteral("track_id");
const QString LOCATION_ID = QStringLiteral("location_id");

const QString PLAYLISTTABLE_ID = QStringLiteral("id");
const QString PLAYLISTTABLE_NAME = QStringLiteral("name");
const QString PLAYLISTTABLE_POSITION = QStringLiteral("position");
const QString PLAYLISTTABLE_HIDDEN = QStringLiteral("hidden");
const QString PLAYLISTTABLE_DATECREATED = QStringLiteral("date_created");
const QString PLAYLISTTABLE_DATEMODIFIED = QStringLiteral("date_modified");

const QString PLAYLISTTRACKSTABLE_TRACKID = QStringLiteral("track_id");
const QString PLAYLISTTRACKSTABLE_POSITION = QStringLiteral("position");
const QString PLAYLISTTRACKSTABLE_PLAYLISTID = QStringLiteral("playlist_id");
const QString PLAYLISTTRACKSTABLE_DATETIMEADDED = QStringLiteral("pl_datetime_added");

const QString REKORDBOX_ANALYZE_PATH = "analyze_path";

// Fingerprint and CMRT Tables
const QString FINGERPRINT_METADATA_TABLE = QStringLiteral("fingerprint_metadata");
const QString CMRT_GROUPS_TABLE = QStringLiteral("cmrt_groups");

// Columns for fingerprint_metadata
const QString FINGERPRINT_TABLE_TRACK_ID = QStringLiteral("track_id");
const QString FINGERPRINT_TABLE_HASH = QStringLiteral("fingerprint_hash");
const QString FINGERPRINT_TABLE_SHA256 = QStringLiteral("chroma_sha256");
const QString FINGERPRINT_TABLE_DURATION = QStringLiteral("fingerprint_duration");
const QString FINGERPRINT_TABLE_VERSION = QStringLiteral("fingerprint_version");
const QString FINGERPRINT_TABLE_GROUP_ID = QStringLiteral("cmrt_group_id");
const QString FINGERPRINT_TABLE_OFFSET = QStringLiteral("cmrt_offset_seconds");
const QString FINGERPRINT_TABLE_CANONICAL = QStringLiteral("is_canonical");
const QString FINGERPRINT_TABLE_VALID = QStringLiteral("fingerprint_valid");
const QString FINGERPRINT_TABLE_NEEDS_REGEN = QStringLiteral("fingerprint_needs_regen");
const QString FINGERPRINT_TABLE_COMPUTED_AT = QStringLiteral("computed_at");

// Columns for cmrt_groups
const QString CMRT_GROUPS_TABLE_ID = QStringLiteral("group_id");
const QString CMRT_GROUPS_TABLE_HASH = QStringLiteral("fingerprint_hash");
const QString CMRT_GROUPS_TABLE_SHA256 = QStringLiteral("chroma_sha256");
const QString CMRT_GROUPS_TABLE_CANONICAL_TRACK_ID = QStringLiteral("canonical_track_id");
const QString CMRT_GROUPS_TABLE_TRACK_COUNT = QStringLiteral("track_count");
const QString CMRT_GROUPS_TABLE_CREATED_AT = QStringLiteral("created_at");
const QString CMRT_GROUPS_TABLE_LAST_UPDATED = QStringLiteral("last_updated");
// MusicBrainz CMRT fields deferred to later PR

// Virtual columns exposed by library_view (and any other BaseTrackCache-backed
// view that wants the CMRT column) — these come from a self-join of library
// against cmrt_groups/cmrt_members, not from a single physical table. Same
// idea as LIBRARYTABLE_COVERART (coverart_digest AS coverart) already used
// in library_view; see LibraryTableModel::setTableModel().
const QString LIBRARYTABLE_CMRT_NAME = QStringLiteral("cmrt_track_name");
const QString LIBRARYTABLE_CMRT_CANONICAL = QStringLiteral("cmrt_is_canonical");
const QString LIBRARYTABLE_CMRT_OFFSET = QStringLiteral("cmrt_offset");

namespace mixxx {
namespace trackschema {
// TableForColumn returns the name of the table that contains the named column.
QString tableForColumn(const QString& columnName);
} // namespace trackschema
} // namespace mixxx
