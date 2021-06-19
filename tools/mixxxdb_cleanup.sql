-----------------------------------------------------------------------
-- !!! BACKUP YOUR mixxxdb.sqlite BEFORE RUNNING THIS SQL SCRIPT !!! --
-- ...as it will simply delete all inconsistent data.                --
--                                                                   --
-- USAGE (Unix/Bash):                                                --
--   sqlite3 \                                                       --
--         ${HOME}/.mixxx/mixxxdb.sqlite \                           --
--         < mixxxdb_cleanup.sql                                     --
--                                                                   --
-- TODO: This task should actually be integrated into Mixxx.         --
-- The different components should be responsible to keep their      --
-- persistent data consistent and valid. If possible FK constraint   --
-- should be enabled enforced at runtime to prevent such issues in   --
-- the first place!                                                  --
-----------------------------------------------------------------------

-- Foreign key constraints in the Mixxx schema are completely broken
-- and disabled at runtime anyway. They would cause major performance
-- issues during cleanup and would also let some of the following
-- statements fail.
PRAGMA foreign_keys = OFF;

-----------------------------------------------------------------------
-- Pre-cleanup checks                                                --
-----------------------------------------------------------------------

PRAGMA integrity_check;

-----------------------------------------------------------------------
-- Fix referential integrity issues in the Mixxx library             --
-----------------------------------------------------------------------

-- Tracks
DELETE FROM library WHERE location NOT IN (SELECT id FROM track_locations);

-- Cues
DELETE FROM cues WHERE track_id NOT IN (SELECT id FROM library);

-- Crates
DELETE FROM crate_tracks WHERE crate_id NOT IN (SELECT id FROM crates);
DELETE FROM crate_tracks WHERE track_id NOT IN (SELECT id FROM library);

-- Playlists
DELETE FROM PlaylistTracks WHERE playlist_id NOT IN (SELECT id FROM Playlists);
DELETE FROM PlaylistTracks WHERE track_id NOT IN (SELECT id FROM library);

-- Analysis
DELETE FROM track_analysis WHERE track_id NOT IN (SELECT id FROM track_locations);

-- Custom Tags (scheduled for 2.4.0)
--DELETE FROM track_custom_tags WHERE track_id NOT IN (SELECT id FROM library);

-----------------------------------------------------------------------
-- Fix referential integrity issues in external libraries (optional) --
-- Enable conditionally depending on the contents of mixxxdb.sqlite  --
-----------------------------------------------------------------------

-- iTunes
--DELETE FROM itunes_playlist_tracks WHERE playlist_id NOT IN (SELECT id FROM itunes_playlists);
--DELETE FROM itunes_playlist_tracks WHERE track_id NOT IN (SELECT id FROM itunes_library);

-- Rekordbox
--DELETE FROM rekordbox_playlist_tracks WHERE playlist_id NOT IN (SELECT id FROM rekordbox_playlists);
--DELETE FROM rekordbox_playlist_tracks WHERE track_id NOT IN (SELECT id FROM rekordbox_library);

-- Rhythmbox
--DELETE FROM rhythmbox_playlist_tracks WHERE playlist_id NOT IN (SELECT id FROM rhythmbox_playlists);
--DELETE FROM rhythmbox_playlist_tracks WHERE track_id NOT IN (SELECT id FROM rhythmbox_playlists);

-- Traktor
--DELETE FROM traktor_playlist_tracks WHERE playlist_id NOT IN (SELECT id FROM traktor_playlists);
--DELETE FROM traktor_playlist_tracks WHERE track_id NOT IN (SELECT id FROM traktor_playlists);

-- Serato
--DELETE FROM serato_playlist_tracks WHERE playlist_id NOT IN (SELECT id FROM serato_playlists);
--DELETE FROM serato_playlist_tracks WHERE track_id NOT IN (SELECT id FROM serato_playlists);

-----------------------------------------------------------------------
-- Post-cleanup maintenance                                          --
-----------------------------------------------------------------------

VACUUM;

PRAGMA optimize;
