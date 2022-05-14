#include "mixer/stem.h"

#include <QRegularExpression>

#include "moc_stem.cpp"
#include "track/track.h"

namespace {

const QRegularExpression kStemRegex(QStringLiteral("^\\[Stem(\\d+)\\]$"));

int extractIntFromRegex(const QRegularExpression& regex, const QString& group) {
    const QRegularExpressionMatch match = regex.match(group);
    DEBUG_ASSERT(match.isValid());
    if (!match.hasMatch()) {
        return false;
    }
    // The regex is expected to contain a single capture group with the number
    constexpr int capturedNumberIndex = 1;
    DEBUG_ASSERT(match.lastCapturedIndex() <= capturedNumberIndex);
    if (match.lastCapturedIndex() < capturedNumberIndex) {
        qWarning() << "No number found in group" << group;
        return false;
    }
    const QString capturedNumber = match.captured(capturedNumberIndex);
    DEBUG_ASSERT(!capturedNumber.isNull());
    bool okay = false;
    const int numberFromMatch = capturedNumber.toInt(&okay);
    VERIFY_OR_DEBUG_ASSERT(okay) {
        return false;
    }
    return numberFromMatch;
}

} //anonymous namespace

Stem::Stem(PlayerManager* pParent,
        UserSettingsPointer pConfig,
        EngineMaster* pMixingEngine,
        EffectsManager* pEffectsManager,
        EngineChannel::ChannelOrientation defaultOrientation,
        const ChannelHandleAndGroup& handleGroup)
        : BaseTrackPlayerImpl(pParent,
                  pConfig,
                  pMixingEngine,
                  pEffectsManager,
                  defaultOrientation,
                  handleGroup,
                  /*defaultMaster*/ true,
                  /*defaultHeadphones*/ false,
                  /*primaryDeck*/ false) {
    stemName = handleGroup.name();
}

void Stem::slotStemPlay(TrackPointer pTrack) {
    int stemNumber = extractIntFromRegex(kStemRegex, stemName);
    QString deckNumber;

    if (stemNumber <= 5) {
        deckNumber = "1";
    }

    else if (stemNumber > 5 && stemNumber <= 10) {
        deckNumber = "2";
    }

    else if (stemNumber > 10 && stemNumber <= 15) {
        deckNumber = "3";
    }

    else if (stemNumber > 15 && stemNumber <= 20) {
        deckNumber = "4";
    }

    ControlProxy* m_DeckPlayPosition = new ControlProxy(
            QString("[Channel") + deckNumber + QString("]"), "playposition");
    ControlProxy* m_DeckVolume = new ControlProxy(
            QString("[Channel") + deckNumber + QString("]"), "volume");
    ControlProxy* m_DeckFileBpm = new ControlProxy(
            QString("[Channel") + deckNumber + QString("]"), "file_bpm");
    ControlProxy* m_DeckBpm = new ControlProxy(
            QString("[Channel") + deckNumber + QString("]"), "bpm");

    ControlProxy* m_StemPlayPosition = new ControlProxy(stemName, "playposition");
    ControlProxy* m_StemVolume = new ControlProxy(stemName, "volume");
    ControlProxy* m_StemPlay = new ControlProxy(stemName, "play");
    ControlProxy* m_StemBpm = new ControlProxy(stemName, "bpm");

    if (pTrack->trySetBpm(m_DeckFileBpm->get())) {
        m_StemBpm->set(m_DeckBpm->get());
    }

    m_StemPlayPosition->set(m_DeckPlayPosition->get());
    m_StemVolume->set(0.4);
    m_StemPlay->set(1.0);

    //m_DeckVolume->set(0.0);
    delete m_DeckPlayPosition;
    delete m_DeckVolume;
    delete m_DeckFileBpm;
    delete m_DeckBpm;
    delete m_StemPlayPosition;
    delete m_StemVolume;
    delete m_StemPlay;
    delete m_StemBpm;
}
