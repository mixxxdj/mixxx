#include "test/signalpathtest.h"

const char* SignalPathTest::m_sMasterGroup = "[Master]";
const char* SignalPathTest::m_sInternalClockGroup = "[InternalClock]";
// these names need to match PlayerManager::groupForDeck and friends
const char* SignalPathTest::m_sGroup1 = "[Channel1]";
const char* SignalPathTest::m_sGroup2 = "[Channel2]";
const char* SignalPathTest::m_sGroup3 = "[Channel3]";
const char* SignalPathTest::m_sPreviewGroup = "[PreviewDeck1]";
const char* SignalPathTest::m_sSamplerGroup = "[Sampler1]";
const double SignalPathTest::kDefaultRateRange = 4.0;
const double SignalPathTest::kDefaultRateDir = 1.0;
const double SignalPathTest::kRateRangeDivisor = kDefaultRateDir * kDefaultRateRange;
const int SignalPathTest::kProcessBufferSize = 1024;
