#pragma once

#include "effects/backends/effectsbackend.h"
#include "effects/defs.h"

/// Refer to EffectsBackend for documentation
class BuiltInBackend : public EffectsBackend {
  public:
    BuiltInBackend();
    virtual ~BuiltInBackend();

    EffectBackendType getType() const {
        return EffectBackendType::BuiltIn;
    };

    const QList<QString> getEffectIds() const;
    EffectManifestPointer getManifest(const QString& effectId) const;
    const QList<EffectManifestPointer> getManifests() const;
    std::unique_ptr<EffectProcessor> createProcessor(
            const EffectManifestPointer pManifest) const;
    bool canInstantiateEffect(const QString& effectId) const;

  private:
    QString debugString() const {
        return "BuiltInBackend";
    }

    typedef std::unique_ptr<EffectProcessor> (*EffectProcessorInstantiator)();

    struct RegisteredEffect {
        EffectManifestPointer pManifest;
        EffectProcessorInstantiator instantiator;
    };

    void registerEffectInner(const QString& id,
            EffectManifestPointer pManifest,
            EffectProcessorInstantiator instantiator);

    template<typename EffectProcessorImpl>
    void registerEffect() {
        registerEffectInner(
                EffectProcessorImpl::getId(),
                EffectProcessorImpl::getManifest(),
                []() {
                    return static_cast<std::unique_ptr<EffectProcessor>>(
                            std::make_unique<EffectProcessorImpl>());
                });
    };

    QMap<QString, RegisteredEffect> m_registeredEffects;
    QList<QString> m_effectIds;
};
