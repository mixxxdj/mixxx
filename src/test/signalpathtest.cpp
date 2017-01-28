#include "test/signalpathtest.h"

const char* BaseSignalPathTest::m_sMasterGroup = "[Master]";
const char* BaseSignalPathTest::m_sInternalClockGroup = "[InternalClock]";
// these names need to match PlayerManager::groupForDeck and friends
const char* BaseSignalPathTest::m_sGroup1 = "[Channel1]";
const char* BaseSignalPathTest::m_sGroup2 = "[Channel2]";
const char* BaseSignalPathTest::m_sGroup3 = "[Channel3]";
const char* BaseSignalPathTest::m_sPreviewGroup = "[PreviewDeck1]";
const char* BaseSignalPathTest::m_sSamplerGroup = "[Sampler1]";
const double BaseSignalPathTest::kDefaultRateRange = 4.0;
const double BaseSignalPathTest::kDefaultRateDir = 1.0;
const double BaseSignalPathTest::kRateRangeDivisor = kDefaultRateDir * kDefaultRateRange;
const int BaseSignalPathTest::kProcessBufferSize = 1024;
