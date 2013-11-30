#ifndef EFFECTCHAIN_H
#define EFFECTCHAIN_H

#include <QObject>
#include <QMap>
#include <QList>

#include "util.h"
#include "effects/effect.h"

class EffectsManager;
class EngineEffectChain;
class EffectChain;
typedef QSharedPointer<EffectChain> EffectChainPointer;

// The main-thread representation of an effect chain. This class is NOT
// thread-safe and must only be used from the main thread.
class EffectChain : public QObject {
    Q_OBJECT
  public:
    EffectChain(EffectsManager* pEffectsManager, const QString& id);
    virtual ~EffectChain();

    // The ID of an EffectChain is a unique ID given to it to help associate it
    // with the preset from which it was loaded.
    QString id() const;

    // Whether the chain is enabled (loaded to a slot and eligible for
    // processing).
    bool enabled() const;
    void setEnabled(bool enabled);

    // Activates EffectChain processing for the provided group.
    void enableForGroup(const QString& group);
    bool enabledForGroup(const QString& group) const;
    void disableForGroup(const QString& group);

    // Get the human-readable name of the EffectChain
    QString name() const;
    void setName(const QString& name);

    double parameter() const;
    void setParameter(const double& dParameter);

    double mix() const;
    void setMix(const double& dMix);

    void addEffect(EffectPointer pEffect);
    void removeEffect(EffectPointer pEffect);
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

    EffectsManager* m_pEffectsManager;

    bool m_bEnabled;
    QString m_id;
    QString m_name;
    double m_dMix;
    double m_dParameter;

    QSet<QString> m_enabledGroups;
    QList<EffectPointer> m_effects;
    EngineEffectChain* m_pEngineEffectChain;

    DISALLOW_COPY_AND_ASSIGN(EffectChain);
};

#endif /* EFFECTCHAIN_H */
