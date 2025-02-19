#pragma once

#include <QString>

#define SEARCHCRATESTABLE "searchcrates"
#define SEARCHCRATETRACKSTABLE "searchcrate_tracks"

const QString SEARCHCRATESTABLE_ID = "id";
const QString SEARCHCRATESTABLE_NAME = "name";
// TODO(XXX): Fix AutoDJ database design.
// SearchCrate should have no dependency on AutoDJ stuff. Which
// searchCrate are used as a source for AutoDJ has to be stored
// and managed by the AutoDJ component in a separate table.
// This refactoring should be deferred until consensus on the
// redesign of the AutoDJ feature has been reached. The main
// ideas of the new design should be documented for verification
// before starting to code.
const QString SEARCHCRATESTABLE_AUTODJ_SOURCE = "autodj_source";
const QString SEARCHCRATESTABLE_SEARCH_INPUT = "search_input";
const QString SEARCHCRATESTABLE_SEARCH_SQL = "search_sql";

// const QString SEARCHCRATESTABLE_COUNT = "count";
// const QString SEARCHCRATESTABLE_SHOW = "show";
// const QString SEARCHCRATESTABLE_LOCKED = "locked";

const QString SEARCHCRATESTABLE_CONDITION_1_FIELD = "condition1_field";
const QString SEARCHCRATESTABLE_CONDITION_1_OPERATOR = "condition1_operator";
const QString SEARCHCRATESTABLE_CONDITION_1_VALUE = "condition1_value";
const QString SEARCHCRATESTABLE_CONDITION_1_COMBINER = "condition1_combiner";

const QString SEARCHCRATESTABLE_CONDITION_2_FIELD = "condition2_field";
const QString SEARCHCRATESTABLE_CONDITION_2_OPERATOR = "condition2_operator";
const QString SEARCHCRATESTABLE_CONDITION_2_VALUE = "condition2_value";
const QString SEARCHCRATESTABLE_CONDITION_2_COMBINER = "condition2_combiner";

const QString SEARCHCRATESTABLE_CONDITION_3_FIELD = "condition3_field";
const QString SEARCHCRATESTABLE_CONDITION_3_OPERATOR = "condition3_operator";
const QString SEARCHCRATESTABLE_CONDITION_3_VALUE = "condition3_value";
const QString SEARCHCRATESTABLE_CONDITION_3_COMBINER = "condition3_combiner";

const QString SEARCHCRATESTABLE_CONDITION_4_FIELD = "condition4_field";
const QString SEARCHCRATESTABLE_CONDITION_4_OPERATOR = "condition4_operator";
const QString SEARCHCRATESTABLE_CONDITION_4_VALUE = "condition4_value";
const QString SEARCHCRATESTABLE_CONDITION_4_COMBINER = "condition4_combiner";

const QString SEARCHCRATESTABLE_CONDITION_5_FIELD = "condition5_field";
const QString SEARCHCRATESTABLE_CONDITION_5_OPERATOR = "condition5_operator";
const QString SEARCHCRATESTABLE_CONDITION_5_VALUE = "condition5_value";
const QString SEARCHCRATESTABLE_CONDITION_5_COMBINER = "condition5_combiner";

const QString SEARCHCRATESTABLE_CONDITION_6_FIELD = "condition6_field";
const QString SEARCHCRATESTABLE_CONDITION_6_OPERATOR = "condition6_operator";
const QString SEARCHCRATESTABLE_CONDITION_6_VALUE = "condition6_value";
const QString SEARCHCRATESTABLE_CONDITION_6_COMBINER = "condition6_combiner";

const QString SEARCHCRATESTABLE_CONDITION_7_FIELD = "condition7_field";
const QString SEARCHCRATESTABLE_CONDITION_7_OPERATOR = "condition7_operator";
const QString SEARCHCRATESTABLE_CONDITION_7_VALUE = "condition7_value";
const QString SEARCHCRATESTABLE_CONDITION_7_COMBINER = "condition7_combiner";

const QString SEARCHCRATESTABLE_CONDITION_8_FIELD = "condition8_field";
const QString SEARCHCRATESTABLE_CONDITION_8_OPERATOR = "condition8_operator";
const QString SEARCHCRATESTABLE_CONDITION_8_VALUE = "condition8_value";
const QString SEARCHCRATESTABLE_CONDITION_8_COMBINER = "condition8_combiner";

const QString SEARCHCRATESTABLE_CONDITION_9_FIELD = "condition9_field";
const QString SEARCHCRATESTABLE_CONDITION_9_OPERATOR = "condition9_operator";
const QString SEARCHCRATESTABLE_CONDITION_9_VALUE = "condition9_value";
const QString SEARCHCRATESTABLE_CONDITION_9_COMBINER = "condition9_combiner";

const QString SEARCHCRATESTABLE_CONDITION_10_FIELD = "condition10_field";
const QString SEARCHCRATESTABLE_CONDITION_10_OPERATOR = "condition10_operator";
const QString SEARCHCRATESTABLE_CONDITION_10_VALUE = "condition10_value";
const QString SEARCHCRATESTABLE_CONDITION_10_COMBINER = "condition10_combiner";

const QString SEARCHCRATESTABLE_CONDITION_11_FIELD = "condition11_field";
const QString SEARCHCRATESTABLE_CONDITION_11_OPERATOR = "condition11_operator";
const QString SEARCHCRATESTABLE_CONDITION_11_VALUE = "condition11_value";
const QString SEARCHCRATESTABLE_CONDITION_11_COMBINER = "condition11_combiner";

const QString SEARCHCRATESTABLE_CONDITION_12_FIELD = "condition12_field";
const QString SEARCHCRATESTABLE_CONDITION_12_OPERATOR = "condition12_operator";
const QString SEARCHCRATESTABLE_CONDITION_12_VALUE = "condition12_value";
const QString SEARCHCRATESTABLE_CONDITION_12_COMBINER = "condition12_combiner";

const QString SEARCHCRATETRACKSTABLE_SEARCHCRATEID = "searchcrate_id";
const QString SEARCHCRATETRACKSTABLE_TRACKID = "track_id";
