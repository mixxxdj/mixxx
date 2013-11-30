#ifndef EFFECTCHAIN_H
#define EFFECTCHAIN_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QList>

#include "util.h"
#include "effects/effect.h"

class EngineEffectChain;
class EffectChain;
typedef QSharedPointer<EffectChain> EffectChainPointer;

// The main-thread representation of an effect chain. This class is NOT
// thread-safe and must only be used from the main thread.
class EffectChain : public QObject {
    Q_OBJECT
  public:
    EffectChain(QObject* pParent, const QString& id);
    virtual ~EffectChain();

    // The ID of an EffectChain is a unique ID given to it to help associate it
    // with the preset from which it was loaded.
    QString id() const;

    // Get the human-readable name of the EffectChain
    QString name() const;
    void setName(const QString& name);

    double parameter() const;
    void setParameter(const double& dParameter);

    void addEffect(EffectPointer pEffect);
    EffectPointer getEffect(unsigned int i) const;
    QList<EffectPointer> getEffects() const;
    unsigned int numEffects() const;

    EngineEffectChain* getEngineEffectChain();

  signals:
    // Signal that indicates that the EffectChain has changed (i.e. an Effect
    // has been added or removed).
    void updated();

  private:
    QString debugString() const {
        return QString("EffectChain(%1)").arg(m_id);
    }

    bool m_bEnabled;
    QString m_id;
    QString m_name;
    double m_dMix;
    double m_dParameter;

    QList<EffectPointer> m_effects;
    EngineEffectChain* m_pEngineEffectChain;

    DISALLOW_COPY_AND_ASSIGN(EffectChain);
};

#endif /* EFFECTCHAIN_H */
