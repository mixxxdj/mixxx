#include "library/trackset/crate/cratesummary.h"

CrateSummary::CrateSummary(CrateId id)
        : Crate(id),
          m_trackCount(0),
          m_trackDuration(0.0) {
}

CrateSummary::~CrateSummary(){};
