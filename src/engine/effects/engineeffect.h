#pragma once

#include <QMap>
#include <QSet>
#include <QString>
#include <QVector>
#include <memory>

#include "audio/types.h"
#include "effects/backends/effectmanifest.h"
#include "effects/backends/effectprocessor.h"
#include "engine/channelhandle.h"
#include "engine/effects/message.h"
#include "util/types.h"

/// EngineEffect is a generic wrapper around an EffectProcessor which intermediates
/// between an EffectSlot and the EffectProcessor. It implements the logic to handle
/// changes of state (enable switch, chain routing switches, parameters' state) so
/// so EffectProcessor subclasses only need to implement their specific DSP logic.
class EngineEffect final : public EffectsRequestHandler {
  public:
    /// Called in main thread by EffectSlot
    EngineEffect(EffectManifestPointer pManifest,
            EffectsBackendManagerPointer pBackendManager,
            const QSet<ChannelHandleAndGroup>& activeInputChannels,
            const QSet<ChannelHandleAndGroup>& registeredInputChannels,
            const QSet<ChannelHandleAndGroup>& registeredOutputChannels);
    /// Called in main thread by EffectSlot
    ~EngineEffect();

    /// Called from the main thread to make sure that the channel already has states
    void initalizeInputChannel(ChannelHandle inputChannel);

    /// Called in audio thread
    bool processEffectsRequest(
            EffectsRequest& message,
            EffectsResponsePipe* pResponsePipe) override;

    /// Called in audio thread
    bool process(const ChannelHandle& inputHandle,
            const ChannelHandle& outputHandle,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const unsigned int numSamples,
            const mixxx::audio::SampleRate sampleRate,
            const EffectEnableState chainEnableState,
            const GroupFeatureState& groupFeatures);

    const EffectManifestPointer getManifest() const {
        return m_pManifest;
    }

    const QString& name() const {
        return m_pManifest->name();
    }

    SINT getGroupDelayFrames() {
        return m_pProcessor->getGroupDelayFrames();
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
