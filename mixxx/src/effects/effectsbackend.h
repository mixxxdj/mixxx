#ifndef EFFECTSBACKEND_H
#define EFFECTSBACKEND_H

#include <QObject>
#include <QList>
#include <QString>

#include "effects/effect.h"

typedef EffectPointer (*EffectInstantiator)(EffectsBackend*, EffectManifestPointer);

// An EffectsBackend is an implementation of a provider of Effect's for use
// within the rest of Mixxx. The job of the EffectsBackend is to both enumerate
// and instantiate effects.
class EffectsBackend : public QObject {
    Q_OBJECT
  public:
    EffectsBackend(QObject* pParent, QString name);
    virtual ~EffectsBackend();

    virtual const QString getName() const;

    virtual const QList<QString> getEffectIds() const;
    virtual EffectManifestPointer getManifest(const QString effectId) const;
    virtual EffectPointer instantiateEffect(const QString effectId);

  protected:
    void registerEffect(const QString id,
                        EffectManifestPointer pManifest,
                        EffectInstantiator pInstantiator);

  private:
    QString m_name;
    QMap<QString, QPair<EffectManifestPointer, EffectInstantiator> > m_registeredEffects;

};

#endif /* EFFECTSBACKEND_H */
