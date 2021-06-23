#pragma once

#include <QSharedPointer>
#include <QDomDocument>

#include "engine/channelhandle.h"
#include "engine/engine.h"
#include "effects/effectmanifest.h"
#include "effects/effectparameter.h"
#include "effects/effectinstantiator.h"
#include "util/class.h"

class EffectState;
class EffectProcessor;
class EngineEffectChain;
class EngineEffect;
class EffectsManager;

// The Effect class is the main-thread representation of an instantiation of an
// effect. This class is NOT thread safe and must only be used by the main
// thread. The getEngineEffect() method can be used to get a pointer to the
// Engine-thread representation of the effect.
class Effect : public QObject {
    Q_OBJECT
  public:
    typedef bool (*ParameterFilterFnc)(EffectParameter*);

    Effect(EffectsManager* pEffectsManager,
           EffectManifestPointer pManifest,
           EffectInstantiatorPointer pInstantiator);
    virtual ~Effect();

    EffectState* createState(const mixxx::EngineParameters& bufferParameters);

    EffectManifestPointer getManifest() const;

    unsigned int numKnobParameters() const;
    unsigned int numButtonParameters() const;

    static bool isButtonParameter(EffectParameter* parameter);
    static bool isKnobParameter(EffectParameter* parameter);

    EffectParameter* getFilteredParameterForSlot(
            ParameterFilterFnc filterFnc, unsigned int slotNumber);
    EffectParameter* getKnobParameterForSlot(unsigned int slotNumber);
    EffectParameter* getButtonParameterForSlot(unsigned int slotNumber);

    EffectParameter* getParameterById(const QString& id) const;
    EffectParameter* getButtonParameterById(const QString& id) const;

    void setEnabled(bool enabled);
    bool enabled() const;

    EngineEffect* getEngineEffect();

    void addToEngine(EngineEffectChain* pChain, int iIndex,
                     const QSet<ChannelHandleAndGroup>& activeInputChannels);
    void removeFromEngine(EngineEffectChain* pChain, int iIndex);
    void updateEngineState();

    static EffectPointer createFromXml(EffectsManager* pEffectsManager,
                                 const QDomElement& element);

    double getMetaknobDefault();

  signals:
    void enabledChanged(bool enabled);

  private:
    QString debugString() const {
        return QString("Effect(%1)").arg(m_pManifest->name());
    }

    void sendParameterUpdate();

    EffectsManager* m_pEffectsManager;
    EffectManifestPointer m_pManifest;
    EffectInstantiatorPointer m_pInstantiator;
    EngineEffect* m_pEngineEffect;
    bool m_bAddedToEngine;
    bool m_bEnabled;
    QList<EffectParameter*> m_parameters;
    QMap<QString, EffectParameter*> m_parametersById;

    DISALLOW_COPY_AND_ASSIGN(Effect);
};
