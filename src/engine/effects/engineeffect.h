#pragma once

#include <QMap>
#include <QString>
#include <QList>
#include <QVector>
#include <QSet>
#include <QtDebug>

#include "effects/effectsmanager.h"
#include "effects/effectmanifest.h"
#include "effects/effectprocessor.h"
#include "effects/effectinstantiator.h"
#include "engine/channelhandle.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/effects/message.h"
#include "engine/effects/groupfeaturestate.h"

class EngineEffect : public EffectsRequestHandler {
  public:
    EngineEffect(EffectManifestPointer pManifest,
                 const QSet<ChannelHandleAndGroup>& activeInputChannels,
                 EffectsManager* pEffectsManager,
                 EffectInstantiatorPointer pInstantiator);
    virtual ~EngineEffect();

    const QString& name() const {
        return m_pManifest->name();
    }

    EngineEffectParameter* getParameterById(const QString& id) {
        return m_parametersById.value(id, NULL);
    }

    EffectState* createState(const mixxx::EngineParameters& bufferParameters);

    void loadStatesForInputChannel(const ChannelHandle* inputChannel,
      EffectStatesMap* pStatesMap);
    void deleteStatesForInputChannel(const ChannelHandle* inputChannel);

    bool processEffectsRequest(
        EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe);

    bool process(const ChannelHandle& inputHandle, const ChannelHandle& outputHandle,
                 const CSAMPLE* pInput, CSAMPLE* pOutput,
                 const unsigned int numSamples,
                 const unsigned int sampleRate,
                 const EffectEnableState chainEnableState,
                 const GroupFeatureState& groupFeatures);

    const EffectManifestPointer getManifest() const {
        return m_pManifest;
    }

  private:
    QString debugString() const {
        return QString("EngineEffect(%1)").arg(m_pManifest->name());
    }

    EffectManifestPointer m_pManifest;
    EffectProcessor* m_pProcessor;
    ChannelHandleMap<ChannelHandleMap<EffectEnableState>> m_effectEnableStateForChannelMatrix;
    bool m_effectRampsFromDry;
    // Must not be modified after construction.
    QVector<EngineEffectParameter*> m_parameters;
    QMap<QString, EngineEffectParameter*> m_parametersById;

    const EffectsManager* m_pEffectsManager;

    DISALLOW_COPY_AND_ASSIGN(EngineEffect);
};
