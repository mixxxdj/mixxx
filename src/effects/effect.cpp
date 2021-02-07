#include "effects/effect.h"

#include <QtDebug>

#include "effects/effectprocessor.h"
#include "effects/effectsmanager.h"
#include "effects/effectxmlelements.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectchain.h"
#include "moc_effect.cpp"
#include "util/xml.h"

Effect::Effect(EffectsManager* pEffectsManager,
        EffectManifestPointer pManifest,
        EffectInstantiatorPointer pInstantiator)
        : QObject(), // no parent
          m_pEffectsManager(pEffectsManager),
          m_pManifest(pManifest),
          m_pInstantiator(pInstantiator),
          m_pEngineEffect(nullptr),
          m_bAddedToEngine(false),
          m_bEnabled(false) {
    for (const auto& pManifestParameter: m_pManifest->parameters()) {
        EffectParameter* pParameter = new EffectParameter(
                this, pEffectsManager, m_parameters.size(), pManifestParameter);
        m_parameters.append(pParameter);
        if (m_parametersById.contains(pParameter->id())) {
            qWarning() << debugString() << "WARNING: Loaded EffectManifest that had parameters with duplicate IDs. Dropping one of them.";
        }
        m_parametersById[pParameter->id()] = pParameter;
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

EffectState* Effect::createState(const mixxx::EngineParameters& bufferParameters) {
    return m_pEngineEffect->createState(bufferParameters);
}

void Effect::addToEngine(EngineEffectChain* pChain, int iIndex,
                         const QSet<ChannelHandleAndGroup>& activeInputChannels) {
    VERIFY_OR_DEBUG_ASSERT(pChain) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pEngineEffect == nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(!m_bAddedToEngine) {
        return;
    }

    m_pEngineEffect = new EngineEffect(m_pManifest,
            activeInputChannels,
            m_pEffectsManager,
            m_pInstantiator);

    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::ADD_EFFECT_TO_CHAIN;
    request->pTargetChain = pChain;
    request->AddEffectToChain.pEffect = m_pEngineEffect;
    request->AddEffectToChain.iIndex = iIndex;
    m_pEffectsManager->writeRequest(request);

    m_bAddedToEngine = true;
}

void Effect::removeFromEngine(EngineEffectChain* pChain, int iIndex) {
    VERIFY_OR_DEBUG_ASSERT(pChain) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pEngineEffect != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_bAddedToEngine) {
        return;
    }

    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::REMOVE_EFFECT_FROM_CHAIN;
    request->pTargetChain = pChain;
    request->RemoveEffectFromChain.pEffect = m_pEngineEffect;
    request->RemoveEffectFromChain.iIndex = iIndex;
    m_pEffectsManager->writeRequest(request);
    m_pEngineEffect = nullptr;

    m_bAddedToEngine = false;
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

EffectManifestPointer Effect::getManifest() const {
    return m_pManifest;
}

void Effect::setEnabled(bool enabled) {
    if (enabled != m_bEnabled) {
        m_bEnabled = enabled;
        updateEngineState();
        emit enabledChanged(m_bEnabled);
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
        if (parameter->manifest()->controlHint() !=
                EffectManifestParameter::ControlHint::TOGGLE_STEPPING) {
            ++num;
        }
    }
    return num;
}

unsigned int Effect::numButtonParameters() const {
    unsigned int num = 0;
    foreach(const EffectParameter* parameter, m_parameters) {
        if (parameter->manifest()->controlHint() ==
                EffectManifestParameter::ControlHint::TOGGLE_STEPPING) {
            ++num;
        }
    }
    return num;
}

EffectParameter* Effect::getParameterById(const QString& id) const {
    EffectParameter* pParameter = m_parametersById.value(id, NULL);
    if (pParameter == nullptr) {
        qWarning() << debugString() << "getParameterById"
                   << "WARNING: parameter for id does not exist:" << id;
    }
    return pParameter;
}

// static
bool Effect::isButtonParameter(EffectParameter* parameter) {
    return  parameter->manifest()->controlHint() ==
            EffectManifestParameter::ControlHint::TOGGLE_STEPPING;
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
    for (const auto& parameter : qAsConst(m_parameters)) {
        if (parameter->manifest()->showInParameterSlot() && filterFnc(parameter)) {
            if(num == slotNumber) {
                return parameter;
            }
            ++num;
        }
    }
    return nullptr;
}

EffectParameter* Effect::getKnobParameterForSlot(unsigned int slotNumber) {
    return getFilteredParameterForSlot(isKnobParameter, slotNumber);
}

EffectParameter* Effect::getButtonParameterForSlot(unsigned int slotNumber) {
    return getFilteredParameterForSlot(isButtonParameter, slotNumber);
}

// static
EffectPointer Effect::createFromXml(EffectsManager* pEffectsManager,
                              const QDomElement& element) {
    // Empty <Effect/> elements are used to preserve chain order
    // when there are empty slots at the beginning of the chain.
    if (!element.hasChildNodes()) {
        return EffectPointer();
    }
    QString effectId = XmlParse::selectNodeQString(element, EffectXml::EffectId);
    EffectPointer pEffect = pEffectsManager->instantiateEffect(effectId);
    return pEffect;
}

double Effect::getMetaknobDefault() {
    return m_pManifest->metaknobDefault();
}
