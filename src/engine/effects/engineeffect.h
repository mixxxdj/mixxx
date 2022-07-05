#pragma once

#include <QList>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVector>
#include <QtDebug>

#include "effects/backends/effectmanifest.h"
#include "effects/backends/effectprocessor.h"
#include "effects/effectsmanager.h"
#include "engine/channelhandle.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/effects/groupfeaturestate.h"
#include "engine/effects/message.h"
#include "util/memory.h"

/// EngineEffect is a generic wrapper around an EffectProcessor which intermediates
/// between an EffectSlot and the EffectProcessor. It implements the logic to handle
/// changes of state (enable switch, chain routing switches, parameters' state) so
/// so EffectProcessor subclasses only need to implement their specific DSP logic.
class EngineEffect final : public EffectsRequestHandler {
  public:
    /// Called in main thread by EffectSlot
    EngineEffect(EffectManifestPointer pManifest,
            EffectsBackendManagerPointer pBackendManager,
            const QSet<GroupHandle>& activeInputChannels,
            const QSet<GroupHandle>& registeredInputChannels,
            const QSet<GroupHandle>& registeredOutputChannels);
    /// Called in main thread by EffectSlot
    ~EngineEffect();

    /// Called in main thread to allocate an EffectState
    EffectState* createState(const mixxx::EngineParameters& engineParameters);

    /// Called in audio thread to load EffectStates received from the main thread
    void loadStatesForInputChannel(GroupHandle inputChannel,
            EffectStatesMap* pStatesMap);
    /// Called from the main thread for garbage collection after an input channel is disabled
    void deleteStatesForInputChannel(GroupHandle inputChannel);

    /// Called in audio thread
    bool processEffectsRequest(
            EffectsRequest& message,
            EffectsResponsePipe* pResponsePipe) override;

    /// Called in audio thread
    bool process(GroupHandle inputHandle,
            GroupHandle outputHandle,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const unsigned int numSamples,
            const unsigned int sampleRate,
            const EffectEnableState chainEnableState,
            const GroupFeatureState& groupFeatures);

    const EffectManifestPointer getManifest() const {
        return m_pManifest;
    }

    const QString& name() const {
        return m_pManifest->name();
    }

  private:
    QString debugString() const {
        return QString("EngineEffect(%1)").arg(m_pManifest->name());
    }

    EffectManifestPointer m_pManifest;
    std::unique_ptr<EffectProcessor> m_pProcessor;
    ChannelHandleMap<ChannelHandleMap<EffectEnableState>> m_effectEnableStateForChannelMatrix;
    bool m_effectRampsFromDry;
    // Must not be modified after construction.
    QVector<EngineEffectParameterPointer> m_parameters;
    QMap<QString, EngineEffectParameterPointer> m_parametersById;

    DISALLOW_COPY_AND_ASSIGN(EngineEffect);
};
