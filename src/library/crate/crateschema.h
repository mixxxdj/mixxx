#ifndef MIXXX_CRATESCHEMA_H
#define MIXXX_CRATESCHEMA_H


#include <QString>


#define CRATE_TABLE "crates"
#define CRATE_TRACKS_TABLE "crate_tracks"

const QString CRATETABLE_ID = "id";

// TODO(XXX): Fix database design flaw.
// Crates should have no dependency on Auto DJ stuff. Which
// crates are used as a source for Auto DJ has to be stored
// and managed by the Auto DJ component in a separate table.
//
// Proposal:
// table:   autodj_crates
// columns: crate_id
const QString CRATETABLE_AUTODJ_SOURCE = "autodj_source";

const QString CRATETRACKSTABLE_CRATEID = "crate_id";
const QString CRATETRACKSTABLE_TRACKID = "track_id";


#endif // MIXXX_CRATESCHEMA_H
