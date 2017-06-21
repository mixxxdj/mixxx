#ifndef MIXXX_CRATESCHEMA_H
#define MIXXX_CRATESCHEMA_H


#include <QString>


#define CRATE_TABLE "crates"
#define CRATE_TRACKS_TABLE "crate_tracks"

const QString CRATETABLE_ID = "id";
const QString CRATETABLE_NAME = "name";

// TODO(XXX): Fix AutoDJ database design.
// Crates should have no dependency on AutoDJ stuff. Which
// crates are used as a source for AutoDJ has to be stored
// and managed by the AutoDJ component in a separate table.
// This refactoring should be deferred until consensus on the
// redesign of the AutoDJ feature has been reached. The main
// ideas of the new design should be documented for verification
// before starting to code.
const QString CRATETABLE_AUTODJ_SOURCE = "autodj_source";

const QString CRATETRACKSTABLE_CRATEID = "crate_id";
const QString CRATETRACKSTABLE_TRACKID = "track_id";


#endif // MIXXX_CRATESCHEMA_H
