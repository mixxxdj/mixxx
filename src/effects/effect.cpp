#include <QtDebug>

#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "effects/effectsmanager.h"
#include "engine/effects/engineeffectchain.h"
#include "engine/effects/engineeffect.h"
#include "xmlparse.h"

Effect::Effect(EffectsManager* pEffectsManager,
               const EffectManifest& manifest,
               EffectInstantiatorPointer pInstantiator)
        : QObject(), // no parent
          m_pEffectsManager(pEffectsManager),
          m_manifest(manifest),
          m_pInstantiator(pInstantiator),
          m_pEngineEffect(NULL),
          m_bEnabled(true) {
    foreach (const EffectManifestParameter& parameter, m_manifest.parameters()) {
        EffectParameter* pParameter = new EffectParameter(
            this, pEffectsManager, m_parameters.size(), parameter);
        m_parameters.append(pParameter);
        if (m_parametersById.contains(parameter.id())) {
            qWarning() << debugString() << "WARNING: Loaded EffectManifest that had parameters with duplicate IDs. Dropping one of them.";
        }
        m_parametersById[parameter.id()] = pParameter;
    }
    //qDebug() << debugString() << "created" << this;
}

Effect::~Effect() {
    //qDebug() << debugString() << "destroyed" << this;
    m_parametersById.clear();
    for (int i = 0; i < m_parameters.size(); ++i) {
        EffectParameter* pParameter = m_parameters.at(i);
        m_parameters[i] = NULL;
        delete pParameter;
    }
}

void Effect::addToEngine(EngineEffectChain* pChain, int iIndex) {
    if (m_pEngineEffect) {
        return;
    }
    m_pEngineEffect = new EngineEffect(m_manifest,
            m_pEffectsManager->registeredGroups(),
            m_pInstantiator);
    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::ADD_EFFECT_TO_CHAIN;
    request->pTargetChain = pChain;
    request->AddEffectToChain.pEffect = m_pEngineEffect;
    request->AddEffectToChain.iIndex = iIndex;
    m_pEffectsManager->writeRequest(request);
}

void Effect::removeFromEngine(EngineEffectChain* pChain, int iIndex) {
    if (!m_pEngineEffect) {
        return;
    }
    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::REMOVE_EFFECT_FROM_CHAIN;
    request->pTargetChain = pChain;
    request->RemoveEffectFromChain.pEffect = m_pEngineEffect;
    request->RemoveEffectFromChain.iIndex = iIndex;
    m_pEffectsManager->writeRequest(request);
    m_pEngineEffect = NULL;
}

void Effect::updateEngineState() {
    if (!m_pEngineEffect) {
        return;
    }
    sendParameterUpdate();
    foreach (EffectParameter* pParameter, m_parameters) {
        pParameter->updateEngineState();
    }
}

EngineEffect* Effect::getEngineEffect() {
    return m_pEngineEffect;
}

const EffectManifest& Effect::getManifest() const {
    return m_manifest;
}

void Effect::setEnabled(bool enabled) {
    if (enabled != m_bEnabled) {
        m_bEnabled = enabled;
        updateEngineState();
        emit(enabledChanged(m_bEnabled));
    }
}

bool Effect::enabled() const {
    return m_bEnabled;
}

void Effect::sendParameterUpdate() {
    if (!m_pEngineEffect) {
        return;
    }
    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::SET_EFFECT_PARAMETERS;
    pRequest->pTargetEffect = m_pEngineEffect;
    pRequest->SetEffectParameters.enabled = m_bEnabled;
    m_pEffectsManager->writeRequest(pRequest);
}

unsigned int Effect::numKnobParameters() const {
    unsigned int num = 0;
    foreach(const EffectParameter* parameter, m_parameters) {
        if (parameter->manifest().controlHint() != EffectManifestParameter::CONTROL_TOGGLE_STEPPING) {
            ++num;
        }
    }
    return num;
}

unsigned int Effect::numButtonParameters() const {
    unsigned int num = 0;
    foreach(const EffectParameter* parameter, m_parameters) {
        if (parameter->manifest().controlHint() == EffectManifestParameter::CONTROL_TOGGLE_STEPPING) {
            ++num;
        }
    }
    return num;
}

EffectParameter* Effect::getParameterById(const QString& id) const {
    EffectParameter* pParameter = m_parametersById.value(id, NULL);
    if (pParameter == NULL) {
        qWarning() << debugString() << "getParameterById"
                   << "WARNING: parameter for id does not exist:" << id;
    }
    return pParameter;
}

// static
bool Effect::isButtonParameter(EffectParameter* parameter) {
    return  parameter->manifest().controlHint() ==
            EffectManifestParameter::CONTROL_TOGGLE_STEPPING;
}

// static
bool Effect::isKnobParameter(EffectParameter* parameter) {
    return !isButtonParameter(parameter);
}

EffectParameter* Effect::getFilteredParameterForSlot(ParameterFilterFnc filterFnc,
                                                     unsigned int slotNumber) {
    // It's normal to ask for a parameter that doesn't exist. Callers must check
    // for NULL.
    unsigned int num = 0;
    foreach(EffectParameter* parameter, m_parameters) {
        if (parameter->manifest().showInParameterSlot() && filterFnc(parameter)) {
            if(num == slotNumber) {
                return parameter;
            }
            ++num;
        }
    }
    return NULL;
}

EffectParameter* Effect::getKnobParameterForSlot(unsigned int slotNumber) {
    return getFilteredParameterForSlot(isKnobParameter, slotNumber);
}

EffectParameter* Effect::getButtonParameterForSlot(unsigned int slotNumber) {
    return getFilteredParameterForSlot(isButtonParameter, slotNumber);
}

QDomElement Effect::toXML(QDomDocument* doc) const {
    QDomElement element = doc->createElement("Effect");
    XmlParse::addElement(*doc, element, "Id", m_manifest.id());
    XmlParse::addElement(*doc, element, "Version", m_manifest.version());

    QDomElement parameters = doc->createElement("Parameters");
    foreach (EffectParameter* pParameter, m_parameters) {
        const EffectManifestParameter& parameterManifest =
                pParameter->manifest();
        QDomElement parameter = doc->createElement("Parameter");
        XmlParse::addElement(*doc, parameter, "Id", parameterManifest.id());
        // TODO(rryan): Do smarter QVariant formatting?
        XmlParse::addElement(*doc, parameter, "Value", QString::number(pParameter->getValue()));
        // TODO(rryan): Output link state, etc.
        parameters.appendChild(parameter);
    }
    element.appendChild(parameters);

    return element;
}

// static
EffectPointer Effect::fromXML(EffectsManager* pEffectsManager,
                              const QDomElement& element) {
    QString effectId = XmlParse::selectNodeQString(element, "Id");
    EffectPointer pEffect = pEffectsManager->instantiateEffect(effectId);
    // TODO(rryan): Load parameter values / etc. from element.
    return pEffect;
}
