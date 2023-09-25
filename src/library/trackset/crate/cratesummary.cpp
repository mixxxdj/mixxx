#include "library/trackset/crate/cratesummary.h"

#include "moc_cratesummary.cpp"

CrateSummaryWrapper::CrateSummaryWrapper(CrateSummary& summary)
        : QObject(nullptr),
          m_summary(summary) {
}

CrateSummaryWrapper::~CrateSummaryWrapper(){};
