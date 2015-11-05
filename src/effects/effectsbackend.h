#ifndef EFFECTSBACKEND_H
#define EFFECTSBACKEND_H

#include <QObject>
#include <QList>
#include <QSet>
#include <QString>

#include "effects/effect.h"
#include "effects/effectinstantiator.h"

class EffectsManager;
class EffectsBackend;
class EffectProcessor;

// An EffectsBackend is an implementation of a provider of Effect's for use
// within the rest of Mixxx. The job of the EffectsBackend is to both enumerate
// and instantiate effects.
class EffectsBackend : public QObject {
    Q_OBJECT
  public:
    EffectsBackend(QObject* pParent, QString name);
    virtual ~EffectsBackend();

    virtual const QString getName() const;

    // returns a list sorted like it should be displayed in the GUI 
    virtual const QList<QString>& getEffectIds() const;
    virtual EffectManifest getManifest(const QString& effectId) const;
    virtual bool canInstantiateEffect(const QString& effectId) const;
    virtual EffectPointer instantiateEffect(
            EffectsManager* pEffectsManager, const QString& effectId);

  signals:
    void effectRegistered();

  protected:
    void registerEffect(const QString& id,
                        const EffectManifest& manifest,
                        EffectInstantiatorPointer pInstantiator);

    template <typename EffectProcessorImpl>
    void registerEffect() {
        registerEffect(
                EffectProcessorImpl::getId(),
                EffectProcessorImpl::getManifest(),
                EffectInstantiatorPointer(
                            new EffectProcessorInstantiator<EffectProcessorImpl>()));
    }

  private:
    QString m_name;
    QMap<QString, QPair<EffectManifest, EffectInstantiatorPointer> > m_registeredEffects;
    QList<QString> m_effectIds;
};

#endif /* EFFECTSBACKEND_H */
