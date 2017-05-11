#ifndef ENGINEEFFECT_H
#define ENGINEEFFECT_H

#include <QMap>
#include <QString>
#include <QList>
#include <QVector>
#include <QSet>
#include <QtDebug>

#include "effects/effectmanifest.h"
#include "effects/effectprocessor.h"
#include "effects/effectinstantiator.h"
#include "engine/channelhandle.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/effects/message.h"
#include "engine/effects/groupfeaturestate.h"

class EngineEffect : public EffectsRequestHandler {
  public:
    EngineEffect(const EffectManifest& manifest,
                 const QSet<ChannelHandleAndGroup>& registeredChannels,
                 EffectInstantiatorPointer pInstantiator);
    virtual ~EngineEffect();

    const QString& name() const {
        return m_manifest.name();
    }

    EngineEffectParameter* getParameterById(const QString& id) {
        return m_parametersById.value(id, NULL);
    }

    bool processEffectsRequest(
        const EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe);

    void process(const ChannelHandle& handle,
                 const CSAMPLE* pInput, CSAMPLE* pOutput,
                 const unsigned int numSamples,
                 const unsigned int sampleRate,
                 const EffectProcessor::EnableState enableState,
                 const GroupFeatureState& groupFeatures);

    bool disabled() const {
        return m_enableState == EffectProcessor::DISABLED;
    }

  private:
    QString debugString() const {
        return QString("EngineEffect(%1)").arg(m_manifest.name());
    }

    EffectManifest m_manifest;
    EffectProcessor* m_pProcessor;
    EffectProcessor::EnableState m_enableState;
    bool m_effectRampsFromDry;
    // Must not be modified after construction.
    QVector<EngineEffectParameter*> m_parameters;
    QMap<QString, EngineEffectParameter*> m_parametersById;

    DISALLOW_COPY_AND_ASSIGN(EngineEffect);
};

#endif /* ENGINEEFFECT_H */
