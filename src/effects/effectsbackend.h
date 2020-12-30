#pragma once

#include <QObject>
#include <QList>
#include <QSet>
#include <QString>

#include "effects/defs.h"
#include "effects/effect.h"
#include "effects/effectinstantiator.h"
#include "preferences/usersettings.h"

class EffectsManager;
class EffectsBackend;
class EffectProcessor;

// An EffectsBackend is an implementation of a provider of Effect's for use
// within the rest of Mixxx. The job of the EffectsBackend is to both enumerate
// and instantiate effects.
class EffectsBackend : public QObject {
    Q_OBJECT
  public:
    EffectsBackend(QObject* pParent, EffectBackendType type);
    virtual ~EffectsBackend();

    // returns a list sorted like it should be displayed in the GUI 
    virtual const QList<QString> getEffectIds() const;
    virtual EffectManifestPointer getManifest(const QString& effectId) const;
    virtual bool canInstantiateEffect(const QString& effectId) const;
    virtual EffectPointer instantiateEffect(
            EffectsManager* pEffectsManager, const QString& effectId);

  signals:
    void effectRegistered(EffectManifestPointer);

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
