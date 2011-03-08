#ifndef EFFECTSMANAGER_H
#define EFFECTSMANAGER_H

#include <QObject>
#include <QMutex>
#include <QList>
#include <QSet>

#include "util.h"
#include "effects/effect.h"
#include "effects/effectsbackend.h"
#include "effects/effectchainslot.h"
#include "effects/effectchain.h"

class EffectChainManager;

class EffectsManager : public QObject {
    Q_OBJECT
  public:
    EffectsManager(QObject* pParent);
    virtual ~EffectsManager();

    // Add an effect backend to be managed by EffectsManager. EffectsManager
    // takes ownership of the backend, and will delete it when EffectsManager is
    // being deleted. Not thread safe -- use only from the GUI thread.
    void addEffectsBackend(EffectsBackend* pEffectsBackend);

    unsigned int numEffectChainSlots() const;
    void addEffectChainSlot();
    EffectChainSlotPointer getEffectChainSlot(unsigned int i);

    // Take a buffer of numSamples samples of audio from channel channelId,
    // provided as pInput, and apply each EffectChain enabled for this channel
    // to it, putting the resulting output in pOutput. If pInput is equal to
    // pOutput, then the operation must occur in-place. Both pInput and pOutput
    // are represented as stereo interleaved samples. There are numSamples total
    // samples, so numSamples/2 left channel samples and numSamples/2 right
    // channel samples.
    virtual void process(const QString channelId,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples);

    void registerChannel(const QString channelID);

    const QSet<QString> getAvailableEffects() const;
    EffectManifestPointer getEffectManifest(const QString effectId) const;
    EffectPointer instantiateEffect(const QString effectId);

    // Temporary, but for setting up all the default EffectChains
    void setupDefaultChains();

  private slots:
    void loadNextChain(const unsigned int iChainSlotNumber, EffectChainPointer pLoadedChain);
    void loadPrevChain(const unsigned int iChainSlotNumber, EffectChainPointer pLoadedChain);

  private:
    QString debugString() const {
        return "EffectsManager";
    }

    mutable QMutex m_mutex;
    EffectChainManager* m_pEffectChainManager;
    QList<EffectsBackend*> m_effectsBackends;
    QList<EffectChainSlotPointer> m_effectChainSlots;
    QSet<QString> m_registeredChannels;

    DISALLOW_COPY_AND_ASSIGN(EffectsManager);
};


#endif /* EFFECTSMANAGER_H */
