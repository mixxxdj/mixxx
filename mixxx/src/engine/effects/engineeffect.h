#ifndef ENGINEEFFECT_H
#define ENGINEEFFECT_H

#include <QMap>
#include <QString>
#include <QList>
#include <QVector>

#include "effects/effectmanifest.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffectparameter.h"

class EngineEffect {
  public:
    EngineEffect(const EffectManifest& manifest, EffectProcessor* pProcessor)
            : m_manifest(manifest),
              m_pProcessor(pProcessor),
              m_parameters(manifest.parameters().size()) {
        const QList<EffectManifestParameter>& parameters = m_manifest.parameters();
        for (int i = 0; i < parameters.size(); ++i) {
            const EffectManifestParameter& parameter = parameters.at(i);
            EngineEffectParameter* pParameter =
                    new EngineEffectParameter(parameter);
            m_parameters[i] = pParameter;
            m_parametersById[parameter.id()] = pParameter;
        }
    }

    virtual ~EngineEffect() {
        delete m_pProcessor;
        m_parametersById.clear();
        for (int i = 0; i < m_parameters.size(); ++i) {
            EngineEffectParameter* pParameter = m_parameters.at(i);
            m_parameters[i] = NULL;
            delete pParameter;
        }
    }

  private:
    EffectManifest m_manifest;
    EffectProcessor* m_pProcessor;
    QVector<EngineEffectParameter*> m_parameters;
    QMap<QString, EngineEffectParameter*> m_parametersById;
};

#endif /* ENGINEEFFECT_H */
