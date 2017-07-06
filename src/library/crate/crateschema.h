#ifndef MIXXX_CRATESCHEMA_H
#define MIXXX_CRATESCHEMA_H


#include <QString>


#define CRATE_TABLE "crates"
#define CRATE_TRACKS_TABLE "crate_tracks"
#define CRATE_CLOSURE_TABLE "crateClosure"
#define CRATE_PATH_TABLE "cratePath"

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

const QString CLOSURE_PARENTID = "parentId";
const QString CLOSURE_CHILDID = "childId";
const QString CLOSURE_DEPTH = "depth";

const QString PATHTABLE_CRATEID = "crateId";
const QString PATHTABLE_ID_PATH = "idPath";
const QString PATHTABLE_NAME_PATH = "namePath";

#endif // MIXXX_CRATESCHEMA_H
