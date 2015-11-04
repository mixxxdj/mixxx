#include <QtDebug>

#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "effects/effectsmanager.h"
#include "engine/effects/engineeffectchain.h"
#include "engine/effects/engineeffect.h"
#include "xmlparse.h"

Effect::Effect(QObject* pParent, EffectsManager* pEffectsManager,
               const EffectManifest& manifest,
               EffectInstantiatorPointer pInstantiator)
        : QObject(pParent),
          m_pEffectsManager(pEffectsManager),
          m_manifest(manifest),
          m_pEngineEffect(new EngineEffect(manifest,
                                           pEffectsManager->registeredGroups(),
                                           pInstantiator)),
          m_bAddedToEngine(false),
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

    foreach (const EffectManifestParameter& parameter, m_manifest.buttonParameters()) {
        EffectParameter* pParameter = new EffectParameter(
            this, pEffectsManager, m_buttonParameters.size(), parameter);
        m_buttonParameters.append(pParameter);
        if (m_buttonParametersById.contains(parameter.id())) {
            qWarning() << debugString() << "WARNING: Loaded EffectManifest that had parameters with duplicate IDs. Dropping one of them.";
        }
        m_buttonParametersById[parameter.id()] = pParameter;
    }
}

Effect::~Effect() {
    //qDebug() << debugString() << "destroyed";
    m_parametersById.clear();
    for (int i = 0; i < m_parameters.size(); ++i) {
        EffectParameter* pParameter = m_parameters.at(i);
        m_parameters[i] = NULL;
        delete pParameter;
    }
    for (int i = 0; i < m_buttonParameters.size(); ++i) {
        EffectParameter* pParameter = m_buttonParameters.at(i);
        m_buttonParameters[i] = NULL;
        delete pParameter;
    }
}

void Effect::addToEngine(EngineEffectChain* pChain, int iIndex) {
    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::ADD_EFFECT_TO_CHAIN;
    request->pTargetChain = pChain;
    request->AddEffectToChain.pEffect = m_pEngineEffect;
    request->AddEffectToChain.iIndex = iIndex;
    m_pEffectsManager->writeRequest(request);
    m_bAddedToEngine = true;
    foreach (EffectParameter* pParameter, m_parameters) {
        pParameter->addToEngine();
    }

    foreach (EffectParameter* pParameter, m_buttonParameters) {
        pParameter->addToEngine();
    }
}

void Effect::removeFromEngine(EngineEffectChain* pChain, int iIndex) {
    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::REMOVE_EFFECT_FROM_CHAIN;
    request->pTargetChain = pChain;
    request->RemoveEffectFromChain.pEffect = m_pEngineEffect;
    request->RemoveEffectFromChain.iIndex = iIndex;
    m_pEffectsManager->writeRequest(request);
    m_bAddedToEngine = false;
    foreach (EffectParameter* pParameter, m_parameters) {
        pParameter->removeFromEngine();
    }

    foreach (EffectParameter* pParameter, m_buttonParameters) {
        pParameter->removeFromEngine();
    }
}

void Effect::updateEngineState() {
    if (!m_bAddedToEngine) {
        return;
    }
    sendParameterUpdate();
    foreach (EffectParameter* pParameter, m_parameters) {
        pParameter->updateEngineState();
    }

    foreach (EffectParameter* pParameter, m_buttonParameters) {
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
    if (!m_bAddedToEngine) {
        return;
    }
    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::SET_EFFECT_PARAMETERS;
    pRequest->pTargetEffect = m_pEngineEffect;
    pRequest->SetEffectParameters.enabled = m_bEnabled;
    m_pEffectsManager->writeRequest(pRequest);
}

unsigned int Effect::numParameters() const {
    return m_parameters.size();
}

unsigned int Effect::numButtonParameters() const {
    return m_buttonParameters.size();
}

EffectParameter* Effect::getParameterById(const QString& id) const {
    EffectParameter* pParameter = m_parametersById.value(id, NULL);
    if (pParameter == NULL) {
        qWarning() << debugString() << "getParameterById"
                   << "WARNING: parameter for id does not exist:" << id;
    }
    return pParameter;
}

EffectParameter* Effect::getButtonParameterById(const QString& id) const {
    EffectParameter* pParameter = m_buttonParametersById.value(id, NULL);
    if (pParameter == NULL) {
        qWarning() << debugString() << "getParameterById"
                   << "WARNING: parameter for id does not exist:" << id;
    }
    return pParameter;
}

EffectParameter* Effect::getParameter(unsigned int parameterNumber) {
    // It's normal to ask for a parameter that doesn't exist. Callers must check
    // for NULL.
    return m_parameters.value(parameterNumber, NULL);
}

EffectParameter* Effect::getButtonParameter(unsigned int parameterNumber) {
    // It's normal to ask for a parameter that doesn't exist. Callers must check
    // for NULL.
    return m_buttonParameters.value(parameterNumber, NULL);
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
        XmlParse::addElement(*doc, parameter, "Value",
                             pParameter->getValue().toString());
        // TODO(rryan): Output link state, etc.
        parameters.appendChild(parameter);
    }
    element.appendChild(parameters);

    QDomElement buttonParameters = doc->createElement("ButtonParameters");
    foreach (EffectParameter* pParameter, m_buttonParameters) {
        const EffectManifestParameter& parameterManifest =
                pParameter->manifest();
        QDomElement parameter = doc->createElement("ButtonParameter");
        XmlParse::addElement(*doc, parameter, "Id", parameterManifest.id());
        // TODO(rryan): Do smarter QVariant formatting?
        XmlParse::addElement(*doc, parameter, "Value",
                             pParameter->getValue().toString());
        // TODO(rryan): Output link state, etc.
        buttonParameters.appendChild(parameter);
    }
    element.appendChild(buttonParameters);

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
