#ifndef ENGINEEFFECT_H
#define ENGINEEFFECT_H

#include <QMap>
#include <QString>
#include <QList>
#include <QVector>
#include <QSet>
#include <QtDebug>

#include "effects/effectmanifest.h"
#include "effects/effectprocessor.h"
#include "effects/effectinstantiator.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/effects/message.h"
#include "engine/effects/groupfeaturestate.h"

class EngineEffect : public EffectsRequestHandler {
  public:
    EngineEffect(const EffectManifest& manifest,
                 const QSet<QString>& registeredGroups,
                 EffectInstantiatorPointer pInstantiator);
    virtual ~EngineEffect();

    const QString& name() const {
        return m_manifest.name();
    }

    EngineEffectParameter* getParameterById(const QString& id) {
        return m_parametersById.value(id, NULL);
    }

    EngineEffectParameter* getButtonParameterById(const QString& id) {
        return m_buttonParametersById.value(id, NULL);
    }

    bool processEffectsRequest(
        const EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe);

    void process(const QString& group,
                 const CSAMPLE* pInput, CSAMPLE* pOutput,
                 const unsigned int numSamples,
                 const GroupFeatureState& groupFeatures);

    bool enabled() const {
        return m_bEnabled;
    }

  private:
    QString debugString() const {
        return QString("EngineEffect(%1)").arg(m_manifest.name());
    }

    EffectManifest m_manifest;
    EffectProcessor* m_pProcessor;
    bool m_bEnabled;
    // Must not be modified after construction.
    QVector<EngineEffectParameter*> m_parameters;
    QVector<EngineEffectParameter*> m_buttonParameters;
    QMap<QString, EngineEffectParameter*> m_parametersById;
    QMap<QString, EngineEffectParameter*> m_buttonParametersById;
};

#endif /* ENGINEEFFECT_H */
