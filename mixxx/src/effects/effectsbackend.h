#ifndef EFFECTSBACKEND_H
#define EFFECTSBACKEND_H

#include <QObject>
#include <QList>
#include <QString>

#include "effects/effect.h"

// An EffectsBackend is an implementation of a provider of Effect's for use
// within the rest of Mixxx. The job of the EffectsBackend is to both enumerate
// and instantiate effects.
class EffectsBackend : public QObject {
    Q_OBJECT
  public:
    EffectsBackend(QObject* pParent, QString name);
    virtual ~EffectsBackend();

    virtual const QString getName() const;

    virtual const QList<EffectManifestPointer> getAvailableEffects() const = 0;
    virtual EffectPointer instantiateEffect(EffectManifestPointer manifest) = 0;

  private:
    QString m_name;
};

#endif /* EFFECTSBACKEND_H */
