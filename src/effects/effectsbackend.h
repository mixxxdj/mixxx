#ifndef EFFECTSBACKEND_H
#define EFFECTSBACKEND_H

#include <QObject>
#include <QList>
#include <QSet>
#include <QString>

#include "effects/defs.h"
#include "effects/effectslot.h"
#include "effects/effectinstantiator.h"
#include "preferences/usersettings.h"

class EffectsManager;
class EffectsBackend;
class EffectProcessor;

// An EffectsBackend is an implementation of a provider of Effect's for use
// within the rest of Mixxx. The job of the EffectsBackend is to both enumerate
// and instantiate effects.
class EffectsBackend {
  public:
    EffectsBackend(EffectBackendType type);
    virtual ~EffectsBackend();

    EffectBackendType getType() const {
        return m_type;
    }

    // returns a list sorted like it should be displayed in the GUI
    virtual const QList<QString> getEffectIds() const;
    virtual EffectManifestPointer getManifest(const QString& effectId) const;
    virtual EffectInstantiatorPointer getInstantiator(const QString& effectId) const;
    virtual bool canInstantiateEffect(const QString& effectId) const;

  protected:
    void registerEffect(const QString& id,
                        EffectManifestPointer pManifest,
                        EffectInstantiatorPointer pInstantiator);

    template <typename EffectProcessorImpl>
    void registerEffect() {
        registerEffect(
                EffectProcessorImpl::getId(),
                EffectProcessorImpl::getManifest(),
                EffectInstantiatorPointer(
                        new EffectProcessorInstantiator<EffectProcessorImpl>()));
    }

    EffectBackendType m_type;

  private:
    class RegisteredEffect {
      public:
        RegisteredEffect(EffectManifestPointer pManifest, EffectInstantiatorPointer pInitator)
            : m_pManifest(pManifest),
              m_pInitator(pInitator) {
        }

        RegisteredEffect() {
        }

        EffectManifestPointer manifest() const { return m_pManifest; };
        EffectInstantiatorPointer initiator() const { return m_pInitator; };

      private:
        EffectManifestPointer m_pManifest;
        EffectInstantiatorPointer m_pInitator;
    };

    QMap<QString, RegisteredEffect> m_registeredEffects;
    QList<QString> m_effectIds;
};

#endif /* EFFECTSBACKEND_H */
