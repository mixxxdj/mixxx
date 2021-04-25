#include "library/trackset/crate/cratesummary.h"

CrateSummaryWrapper::CrateSummaryWrapper(CrateSummary& summary)
        : QObject(nullptr),
          m_summary(summary) {
}

CrateSummaryWrapper::~CrateSummaryWrapper(){};
