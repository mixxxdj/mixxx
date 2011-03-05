#ifndef EFFECTCHAIN_H
#define EFFECTCHAIN_H

#include <QObject>
#include <QMutex>
#include <QList>

#include "util.h"
#include "effects/effect.h"
#include "effects/effectslot.h"

class EffectChain;
typedef QSharedPointer<EffectChain> EffectChainPointer;

class EffectChain : public QObject {
    Q_OBJECT
  public:
    EffectChain(QObject* pParent, const unsigned int iChainNumber);
    virtual ~EffectChain();

    unsigned int numSlots() const;
    void addEffectSlot();
    EffectSlotPointer getEffectSlot(unsigned int slotNumber);

    bool isEnabled() const;

    // Take a buffer of numSamples samples of audio from channel channelId,
    // provided as pInput, and apply each Effect in this EffectChain to it,
    // putting the resulting output in pOutput. If pInput is equal to pOutput,
    // then the operation must occur in-place. Both pInput and pOutput are
    // represented as stereo interleaved samples. There are numSamples total
    // samples, so numSamples/2 left channel samples and numSamples/2 right
    // channel samples. The channelId provided allows the effects to maintain
    // state on a per-channel basis. This is important because one Effect
    // instance may be used to process the audio of multiple channels.
    virtual void process(const QString channelId,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples);

  signals:
    // Indicates that the effect pEffect has been loaded into slotNumber of
    // EffectChain chainNumber. pEffect may be an invalid pointer, which
    // indicates that a previously loaded effect was removed from the slot.
    void effectLoaded(EffectPointer pEffect, unsigned int chainNumber, unsigned int slotNumber);

    // Signal that whoever is in charge of this EffectChain should load the next
    // preset into it.
    void nextPreset(unsigned int chainNumber);

    // Signal that whoever is in charge of this EffectChain should load the
    // previous preset into it.
    void prevPreset(unsigned int chainNumber);

  private slots:
    void slotEffectLoaded(EffectPointer pEffect, unsigned int slotNumber);

    void slotControlNumEffectsSlots(double v);
    void slotControlChainEnabled(double v);
    void slotControlChainMix(double v);
    void slotControlChainParameter(double v);
    void slotControlChainNextPreset(double v);
    void slotControlChainPrevPreset(double v);

  private:
    QString debugString() const {
        return QString("EffectChain(%1)").arg(m_iChainNumber);
    }

    bool privateIsEnabled() const;

    mutable QMutex m_mutex;
    const unsigned int m_iChainNumber;
    const QString m_group;

    ControlObject* m_pControlNumEffectSlots;
    ControlObject* m_pControlChainEnabled;
    ControlObject* m_pControlChainMix;
    ControlObject* m_pControlChainParameter;
    ControlObject* m_pControlChainNextPreset;
    ControlObject* m_pControlChainPrevPreset;

    QList<EffectSlotPointer> m_slots;

    DISALLOW_COPY_AND_ASSIGN(EffectChain);
};


#endif /* EFFECTCHAIN_H */
