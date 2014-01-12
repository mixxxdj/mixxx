#include "test/mockedenginebackendtest.h"

const char* MockedEngineBackendTest::m_sMasterGroup = "[Master]";
const char* MockedEngineBackendTest::m_sInternalClockGroup = "[InternalClock]";
const char* MockedEngineBackendTest::m_sGroup1 = "[Test1]";
const char* MockedEngineBackendTest::m_sGroup2 = "[Test2]";
const char* MockedEngineBackendTest::m_sGroup3 = "[Test3]";
const double MockedEngineBackendTest::kDefaultRateRange = 4.0;
const double MockedEngineBackendTest::kDefaultRateDir = 1.0;
const double MockedEngineBackendTest::kRateRangeDivisor = kDefaultRateDir * kDefaultRateRange;
