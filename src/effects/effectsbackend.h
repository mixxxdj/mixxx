#ifndef EFFECTSBACKEND_H
#define EFFECTSBACKEND_H

#include <QObject>
#include <QList>
#include <QSet>
#include <QString>

#include "effects/effect.h"

class EffectsManager;
class EffectsBackend;
class EffectInstantiator {
  public:
    virtual ~EffectInstantiator() {}
    virtual EffectPointer instantiate(EffectsManager* pEffectsManager,
                                      EffectsBackend* pEffectsBackend,
                                      const EffectManifest& manifest) = 0;
};

template <typename T>
class EffectProcessorInstantiator : public EffectInstantiator {
  public:
    EffectPointer instantiate(EffectsManager* pEffectsManager,
                              EffectsBackend* pEffectsBackend,
                              const EffectManifest& manifest) {
        return EffectPointer(new Effect(pEffectsBackend, pEffectsManager,
                                        manifest, new T(manifest)));
    }
};


// An EffectsBackend is an implementation of a provider of Effect's for use
// within the rest of Mixxx. The job of the EffectsBackend is to both enumerate
// and instantiate effects.
class EffectsBackend : public QObject {
    Q_OBJECT
  public:
    EffectsBackend(QObject* pParent, QString name);
    virtual ~EffectsBackend();

    virtual const QString getName() const;

    virtual const QSet<QString> getEffectIds() const;
    virtual EffectManifest getManifest(const QString& effectId) const;
    virtual bool canInstantiateEffect(const QString& effectId) const;
    virtual EffectPointer instantiateEffect(
        EffectsManager* pEffectsManager, const QString& effectId);

  protected:
    void registerEffect(const QString& id,
                        const EffectManifest& manifest,
                        EffectInstantiator* pInstantiator);

    template <typename EffectProcessorImpl>
    void registerEffect() {
        registerEffect(EffectProcessorImpl::getId(),
                       EffectProcessorImpl::getManifest(),
                       new EffectProcessorInstantiator<EffectProcessorImpl>());
    }

  private:
    QString m_name;
    QMap<QString, QPair<EffectManifest, EffectInstantiator*> > m_registeredEffects;
};

#endif /* EFFECTSBACKEND_H */
