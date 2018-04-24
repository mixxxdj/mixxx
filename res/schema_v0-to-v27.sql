---------------------------------------------------------------------------------------------------
-- HOWTO: Create a schema migration/initialization file from a fresh mixxxdb.sqlite database
-- 1) Export entire schema into file: echo -e '.output schema_v0-to-v<mixxxdb.version>.sql\n.schema' | sqlite3 "~/.mixxx/mixxxdb.sqlite"
-- 2) Delete create statement(s) for system table(s) named 'sqlite_...'
-- 3) Add insert statements for name/value pairs into table 'settings'
---------------------------------------------------------------------------------------------------

CREATE TABLE settings (
        name TEXT UNIQUE NOT NULL,
        value TEXT,
        locked INTEGER DEFAULT 0,
        hidden INTEGER DEFAULT 0);
INSERT INTO settings (name, value) VALUES ('mixxx.schema.version', 27);
INSERT INTO settings (name, value) VALUES ('mixxx.schema.min_compatible_version', 3);
CREATE TABLE track_locations (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        location varchar(512) UNIQUE,
        filename varchar(512),
        directory varchar(512),
        filesize INTEGER,
        fs_deleted INTEGER,
        needs_verification INTEGER);
CREATE TABLE LibraryHashes (
        directory_path VARCHAR(256) primary key,
        hash INTEGER,
        directory_deleted INTEGER, needs_verification INTEGER DEFAULT 0);
CREATE TABLE Playlists (
        id INTEGER primary key,
        name varchar(48),
        position INTEGER,
        hidden INTEGER DEFAULT 0 NOT NULL,
        date_created datetime,
        date_modified datetime, locked integer DEFAULT 0);
CREATE TABLE PlaylistTracks (
        id INTEGER primary key,
        playlist_id INTEGER REFERENCES Playlists(id),
        track_id INTEGER REFERENCES library(id),
        position INTEGER, pl_datetime_added);
CREATE TABLE cues (
        id integer PRIMARY KEY AUTOINCREMENT,
        track_id integer NOT NULL REFERENCES library(id),
        type integer DEFAULT 0 NOT NULL,
        position integer DEFAULT -1 NOT NULL,
        length integer DEFAULT 0 NOT NULL,
        hotcue integer DEFAULT -1 NOT NULL,
        label text DEFAULT '' NOT NULL, color INTEGER DEFAULT 4294901760 NOT NULL);
CREATE TABLE crates (
        id integer PRIMARY KEY AUTOINCREMENT,
        name varchar(48) UNIQUE NOT NULL,
        count integer DEFAULT 0,
        show integer DEFAULT 1, locked integer DEFAULT 0, autodj_source integer DEFAULT 0);
CREATE TABLE crate_tracks (
        crate_id integer NOT NULL REFERENCES crates(id),
        track_id integer NOT NULL REFERENCES library(id),
        UNIQUE (crate_id, track_id));
CREATE TABLE library (
        id INTEGER primary key AUTOINCREMENT,
        artist varchar(64),
        title varchar(64),
        album varchar(64),
        year varchar(16),
        genre varchar(64),
        tracknumber varchar(3),
        location integer REFERENCES track_locations(location),
        comment varchar(256),
        url varchar(256),
        duration float,
        bitrate integer,
        samplerate integer,
        cuepoint integer,
        bpm float,
        wavesummaryhex blob,
        channels integer,
        datetime_added DEFAULT CURRENT_TIMESTAMP,
        mixxx_deleted integer,
        played integer,
        header_parsed integer DEFAULT 0, filetype varchar(8) DEFAULT "?", replaygain float DEFAULT 0, timesplayed integer DEFAULT 0, rating integer DEFAULT 0, key varchar(8) DEFAULT "", beats BLOB, beats_version TEXT, composer varchar(64) DEFAULT "", bpm_lock INTEGER DEFAULT 0, beats_sub_version TEXT DEFAULT '', keys BLOB, keys_version TEXT, keys_sub_version TEXT, key_id INTEGER DEFAULT 0, grouping TEXT DEFAULT "", album_artist TEXT DEFAULT "", coverart_source INTEGER DEFAULT 0, coverart_type INTEGER DEFAULT 0, coverart_location TEXT DEFAULT "", coverart_hash INTEGER DEFAULT 0, replaygain_peak REAL DEFAULT -1.0, tracktotal TEXT DEFAULT '//');
CREATE TABLE itunes_library (
        id INTEGER primary key,
        artist varchar(48), title varchar(48),
        album varchar(48), year varchar(16),
        genre varchar(32), tracknumber varchar(3),
        location varchar(512),
        comment varchar(60),
        duration integer,
        bitrate integer,
        bpm integer,
        rating integer, grouping TEXT DEFAULT "", album_artist TEXT DEFAULT "");
CREATE TABLE itunes_playlists (
        id INTEGER primary key,
        name varchar(100) UNIQUE);
CREATE TABLE itunes_playlist_tracks (
        id INTEGER primary key AUTOINCREMENT,
        playlist_id INTEGER REFERENCES itunes_playlist(id),
        track_id INTEGER REFERENCES itunes_library(id), position INTEGER DEFAULT 0);
CREATE TABLE traktor_library (
        id INTEGER primary key AUTOINCREMENT,
        artist varchar(48), title varchar(48),
        album varchar(48), year varchar(16),
        genre varchar(32), tracknumber varchar(3),
        location varchar(512) UNIQUE,
        comment varchar(60),
        duration integer,
        bitrate integer,
        bpm float,
        key varchar(6),
        rating integer
        );
CREATE TABLE traktor_playlists (
        id INTEGER primary key,
        name varchar(100) UNIQUE
        );
CREATE TABLE traktor_playlist_tracks (
        id INTEGER primary key AUTOINCREMENT,
        playlist_id INTEGER REFERENCES traktor_playlist(id),
        track_id INTEGER REFERENCES traktor_library(id)
        , position INTEGER DEFAULT 0);
CREATE TABLE rhythmbox_library (
        id INTEGER primary key AUTOINCREMENT,
        artist varchar(48), title varchar(48),
        album varchar(48), year varchar(16),
        genre varchar(32), tracknumber varchar(3),
        location varchar(512) UNIQUE,
        comment varchar(60),
        duration integer,
        bitrate integer,
        bpm float,
        key varchar(6),
        rating integer
        );
CREATE TABLE rhythmbox_playlists (
        id INTEGER primary key AUTOINCREMENT,
        name varchar(100) UNIQUE
        );
CREATE TABLE rhythmbox_playlist_tracks (
        id INTEGER primary key AUTOINCREMENT,
        playlist_id INTEGER REFERENCES rhythmbox_playlist(id),
        track_id INTEGER REFERENCES rhythmbox_library(id)
        , position INTEGER DEFAULT 0);
CREATE TABLE track_analysis (
      id INTEGER primary key AUTOINCREMENT,
      track_id INTEGER NOT NULL REFERENCES track_locations(id),
      type varchar(512),
      description varchar(1024),
      version varchar(512),
      created DEFAULT CURRENT_TIMESTAMP,
      data_checksum varchar(512)
    );
CREATE INDEX track_analysis_track_id_index ON track_analysis (track_id);
CREATE TABLE directories (
        directory TEXT UNIQUE
      );
