#include "test/signalpathtest.h"

const QString BaseSignalPathTest::m_sMasterGroup = QStringLiteral("[Master]");
const QString BaseSignalPathTest::m_sInternalClockGroup = QStringLiteral("[InternalClock]");
// these names need to match PlayerManager::groupForDeck and friends
const QString BaseSignalPathTest::m_sGroup1 = QStringLiteral("[Channel1]");
const QString BaseSignalPathTest::m_sGroup2 = QStringLiteral("[Channel2]");
const QString BaseSignalPathTest::m_sGroup3 = QStringLiteral("[Channel3]");
const QString BaseSignalPathTest::m_sGroup4 = QStringLiteral("[Channel4]");
const QString BaseSignalPathTest::m_sPreviewGroup = QStringLiteral("[PreviewDeck1]");
const QString BaseSignalPathTest::m_sSamplerGroup = QStringLiteral("[Sampler1]");

const double BaseSignalPathTest::kDefaultRateRange = 0.08;
const double BaseSignalPathTest::kDefaultRateDir = 1.0;
const double BaseSignalPathTest::kRateRangeDivisor = kDefaultRateDir * kDefaultRateRange;
const int BaseSignalPathTest::kProcessBufferSize = 1024;
