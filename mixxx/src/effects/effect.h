#ifndef EFFECT_H
#define EFFECT_H

#include <QSharedPointer>
#include <QMutex>

#include "defs.h"
#include "util.h"
#include "effects/effectmanifest.h"
#include "effects/effectparameter.h"

class EffectsBackend;

class Effect;
typedef QSharedPointer<Effect> EffectPointer;

class Effect : public QObject {
    Q_OBJECT
  public:
    Effect(EffectsBackend* pBackend, EffectManifestPointer pManifest);
    virtual ~Effect();

    virtual EffectManifestPointer getManifest() const;

    unsigned int numParameters() const;
    EffectParameterPointer getParameter(unsigned int parameterNumber);

    // Take a buffer of numSamples samples of audio from channel channelId,
    // provided as pInput, process the buffer according to Effect-specific
    // logic, and output it to the buffer pOutput. If pInput is equal to
    // pOutput, then the operation must occur in-place. Both pInput and pOutput
    // are represented as stereo interleaved samples. There are numSamples total
    // samples, so numSamples/2 left channel samples and numSamples/2 right
    // channel samples. The channelId provided allows the effect to maintain
    // state on a per-channel basis. This is important because one Effect
    // instance may be used to process the audio of multiple channels.
    virtual void process(const QString channelId,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples) = 0;

  protected:
    EffectParameterPointer getParameterFromId(const QString id) const;

  private:
    QString debugString() const {
        return QString("Effect(%1)").arg(m_pEffectManifest->name());
    }

    mutable QMutex m_mutex;
    EffectsBackend* m_pEffectsBackend;
    EffectManifestPointer m_pEffectManifest;
    QList<EffectParameterPointer> m_parameters;
    QMap<QString, EffectParameterPointer> m_parametersById;

    DISALLOW_COPY_AND_ASSIGN(Effect);
};

#endif /* EFFECT_H */
