#pragma once

#include <QString>

#define SMARTIES_TABLE "smarties"
#define SMARTIES_TRACKS_TABLE "smarties_tracks"

const QString SMARTIESTABLE_ID = "id";
const QString SMARTIESTABLE_NAME = "name";
// TODO(XXX): Fix AutoDJ database design.
// Smarties should have no dependency on AutoDJ stuff. Which
// smarties are used as a source for AutoDJ has to be stored
// and managed by the AutoDJ component in a separate table.
// This refactoring should be deferred until consensus on the
// redesign of the AutoDJ feature has been reached. The main
// ideas of the new design should be documented for verification
// before starting to code.
const QString SMARTIESTABLE_AUTODJ_SOURCE = "autodj_source";
const QString SMARTIESTABLE_SEARCH_INPUT = "search_input";
const QString SMARTIESTABLE_SEARCH_SQL = "search_sql";

const QString SMARTIESTRACKSTABLE_SMARTIESID = "smarties_id";
const QString SMARTIESTRACKSTABLE_TRACKID = "track_id";
