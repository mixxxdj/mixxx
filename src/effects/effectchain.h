#ifndef EFFECTCHAIN_H
#define EFFECTCHAIN_H

#include <QObject>
#include <QMap>
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
    EffectChain(QObject* pParent=NULL);
    virtual ~EffectChain();

    // The ID of an EffectChain is a unique ID given to it to help associate it
    // with the preset from which it was loaded.
    QString id() const;
    void setId(const QString& id);

    // Get the human-readable name of the EffectChain
    QString name() const;
    void setName(const QString& name);

    double parameter() const;
    void setParameter(const double& dParameter);

    void addEffect(EffectPointer pEffect);
    EffectPointer getEffect(unsigned int i) const;
    QList<EffectPointer> getEffects() const;
    unsigned int numEffects() const;

    // Take a buffer of numSamples samples of audio from channel channelId,
    // provided as pInput, and apply each Effect in this EffectChain to it,
    // putting the resulting output in pOutput. If pInput is equal to pOutput,
    // then the operation must occur in-place. Both pInput and pOutput are
    // represented as stereo interleaved samples. There are numSamples total
    // samples, so numSamples/2 left channel samples and numSamples/2 right
    // channel samples. The channelId provided allows the effects to maintain
    // state on a per-channel basis. This is important because one Effect
    // instance may be used to process the audio of multiple channels.
    virtual void process(const QString& channelId,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples);

  signals:
    // Signal that indicates that the EffectChain has changed (i.e. an Effect
    // has been added or removed).
    void updated();
  private:
    QString debugString() const {
        return QString("EffectChain(%1)").arg(m_id);
    }

    mutable QMutex m_mutex;

    bool m_bEnabled;
    QString m_id;
    QString m_name;
    double m_dMix;
    double m_dParameter;

    QList<EffectPointer> m_effects;

    DISALLOW_COPY_AND_ASSIGN(EffectChain);
};


#endif /* EFFECTCHAIN_H */
