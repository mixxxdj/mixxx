#include "test/mockedenginebackendtest.h"

const char* MockedEngineBackendTest::m_sMasterGroup = "[Master]";
const char* MockedEngineBackendTest::m_sInternalClockGroup = "[InternalClock]";
// these names need to match PlayerManager::groupForDeck and friends
const char* MockedEngineBackendTest::m_sGroup1 = "[Channel1]";
const char* MockedEngineBackendTest::m_sGroup2 = "[Channel2]";
const char* MockedEngineBackendTest::m_sGroup3 = "[Channel3]";
const char* MockedEngineBackendTest::m_sPreviewGroup = "[PreviewDeck1]";
const char* MockedEngineBackendTest::m_sSamplerGroup = "[Sampler1]";
const double MockedEngineBackendTest::kDefaultRateRange = 4.0;
const double MockedEngineBackendTest::kDefaultRateDir = 1.0;
const double MockedEngineBackendTest::kRateRangeDivisor = kDefaultRateDir * kDefaultRateRange;
