#include "library/features/crates/cratestoragehelpers.h"

CrateQueryFields::CrateQueryFields(const FwdSqlQuery& query)
    : m_iId(query.fieldIndex(CRATETABLE_ID)),
      m_iName(query.fieldIndex(CRATETABLE_NAME)),
      m_iLocked(query.fieldIndex(CRATETABLE_LOCKED)),
      m_iAutoDjSource(query.fieldIndex(CRATETABLE_AUTODJ_SOURCE)) {
}


void CrateQueryFields::populateFromQuery(
        const FwdSqlQuery& query,
        Crate* pCrate) const {
    pCrate->setId(getId(query));
    pCrate->setName(getName(query));
    pCrate->setLocked(isLocked(query));
    pCrate->setAutoDjSource(isAutoDjSource(query));
}


CrateTrackQueryFields::CrateTrackQueryFields(const FwdSqlQuery& query)
    : m_iCrateId(query.fieldIndex(CRATETRACKSTABLE_CRATEID)),
      m_iTrackId(query.fieldIndex(CRATETRACKSTABLE_TRACKID)) {
}


CrateSummaryQueryFields::CrateSummaryQueryFields(const FwdSqlQuery& query)
    : CrateQueryFields(query),
      m_iTrackCount(query.fieldIndex(CRATESUMMARY_TRACK_COUNT)),
      m_iTrackDuration(query.fieldIndex(CRATESUMMARY_TRACK_DURATION)) {
}

void CrateSummaryQueryFields::populateFromQuery(
        const FwdSqlQuery& query,
        CrateSummary* pCrateSummary) const {
    CrateQueryFields::populateFromQuery(query, pCrateSummary);
    pCrateSummary->setTrackCount(getTrackCount(query));
    pCrateSummary->setTrackDuration(getTrackDuration(query));
}

