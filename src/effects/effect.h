#ifndef EFFECT_H
#define EFFECT_H

#include <QSharedPointer>
#include <QDomDocument>

#include "util.h"
#include "effects/effectmanifest.h"
#include "effects/effectparameter.h"
#include "effects/effectinstantiator.h"

class EffectProcessor;
class EngineEffectChain;
class EngineEffect;
class EffectsManager;

class Effect;
typedef QSharedPointer<Effect> EffectPointer;

// The Effect class is the main-thread representation of an instantiation of an
// effect. This class is NOT thread safe and must only be used by the main
// thread. The getEngineEffect() method can be used to get a pointer to the
// Engine-thread representation of the effect.
class Effect : public QObject {
    Q_OBJECT
  public:
    Effect(QObject* pParent, EffectsManager* pEffectsManager,
           const EffectManifest& manifest,
           EffectInstantiatorPointer pInstantiator);
    virtual ~Effect();

    const EffectManifest& getManifest() const;

    unsigned int numParameters() const;
    unsigned int numButtonParameters() const;
    EffectParameter* getParameter(unsigned int parameterNumber);
    EffectParameter* getButtonParameter(unsigned int parameterNumber);
    EffectParameter* getParameterById(const QString& id) const;
    EffectParameter* getButtonParameterById(const QString& id) const;

    void setEnabled(bool enabled);
    bool enabled() const;

    EngineEffect* getEngineEffect();

    void addToEngine(EngineEffectChain* pChain, int iIndex);
    void removeFromEngine(EngineEffectChain* pChain, int iIndex);
    void updateEngineState();

    QDomElement toXML(QDomDocument* doc) const;
    static EffectPointer fromXML(EffectsManager* pEffectsManager,
                                 const QDomElement& element);

  signals:
    void enabledChanged(bool enabled);

  private:
    QString debugString() const {
        return QString("Effect(%1)").arg(m_manifest.name());
    }

    void sendParameterUpdate();

    EffectsManager* m_pEffectsManager;
    EffectManifest m_manifest;
    EngineEffect* m_pEngineEffect;
    bool m_bAddedToEngine;
    bool m_bEnabled;
    QList<EffectParameter*> m_parameters;
    QMap<QString, EffectParameter*> m_parametersById;
    QList<EffectParameter*> m_buttonParameters;
    QMap<QString, EffectParameter*> m_buttonParametersById;

    DISALLOW_COPY_AND_ASSIGN(Effect);
};

#endif /* EFFECT_H */
