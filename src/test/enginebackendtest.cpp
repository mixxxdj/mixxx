#include "test/enginebackendtest.h"

const char* EngineBackendTest::m_sMasterGroup = "[Master]";
const char* EngineBackendTest::m_sInternalClockGroup = "[InternalClock]";
// these names need to match PlayerManager::groupForDeck and friends
const char* EngineBackendTest::m_sGroup1 = "[Channel1]";
const char* EngineBackendTest::m_sGroup2 = "[Channel2]";
const char* EngineBackendTest::m_sGroup3 = "[Channel3]";
const char* EngineBackendTest::m_sPreviewGroup = "[PreviewDeck1]";
const char* EngineBackendTest::m_sSamplerGroup = "[Sampler1]";
const double EngineBackendTest::kDefaultRateRange = 4.0;
const double EngineBackendTest::kDefaultRateDir = 1.0;
const double EngineBackendTest::kRateRangeDivisor = kDefaultRateDir * kDefaultRateRange;
const int EngineBackendTest::kProcessBufferSize = 1024;
