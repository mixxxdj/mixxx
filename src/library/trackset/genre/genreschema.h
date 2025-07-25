#pragma once

#include <QString>

#define GENRE_TABLE "genres"
#define GENRE_TRACKS_TABLE "genre_tracks"

const QString GENRETABLE_ID = QStringLiteral("id");
const QString GENRETABLE_NAME = QStringLiteral("name");
// const QString GENRETABLE_NAME = QStringLiteral("name_level_1");
// const QString GENRETABLE_CUSTOMNAME = QStringLiteral("custom_name");

// const QString GENRETABLE_NAME = QStringLiteral("custom_name");

// TODO(XXX): Fix AutoDJ database design.
// Crates should have no dependency on AutoDJ stuff. Which
// crates are used as a source for AutoDJ has to be stored
// and managed by the AutoDJ component in a separate table.
// This refactoring should be deferred until consensus on the
// redesign of the AutoDJ feature has been reached. The main
// ideas of the new design should be documented for verification
// before starting to code.
const QString GENRETABLE_AUTODJ_SOURCE = QStringLiteral("autodj_source");
const QString GENRETABLE_NAMELEVEL1 = QStringLiteral("name_level_1");
const QString GENRETABLE_NAMELEVEL2 = QStringLiteral("name_level_2");
const QString GENRETABLE_NAMELEVEL3 = QStringLiteral("name_level_3");
const QString GENRETABLE_NAMELEVEL4 = QStringLiteral("name_level_4");
const QString GENRETABLE_NAMELEVEL5 = QStringLiteral("name_level_5");
const QString GENRETABLE_DISPLAYGROUP = QStringLiteral("display_group");
const QString GENRETABLE_ISVISIBLE = QStringLiteral("is_visible");
const QString GENRETABLE_ISMODELDEFINED = QStringLiteral("is_model_defined");

const QString GENRETRACKSTABLE_GENREID = QStringLiteral("genre_id");
const QString GENRETRACKSTABLE_TRACKID = QStringLiteral("track_id");
