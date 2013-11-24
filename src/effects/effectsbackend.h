#ifndef EFFECTSBACKEND_H
#define EFFECTSBACKEND_H

#include <QObject>
#include <QList>
#include <QSet>
#include <QString>

#include "effects/effect.h"

class EffectsBackend;
typedef EffectPointer (*EffectInstantiator)(EffectsBackend*, const EffectManifest&);

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
    virtual EffectPointer instantiateEffect(const QString& effectId);

  protected:
    void registerEffect(const QString id,
                        const EffectManifest& manifest,
                        EffectInstantiator pInstantiator);

  private:
    QString m_name;
    QMap<QString, QPair<EffectManifest, EffectInstantiator> > m_registeredEffects;
};

#endif /* EFFECTSBACKEND_H */
