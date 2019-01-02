#include "effects/effectslot.h"
#include "effects/effectxmlelements.h"

#include <QDebug>

#include "effects/defs.h"
#include "control/controlpushbutton.h"
#include "control/controlencoder.h"
#include "control/controlproxy.h"
#include "util/math.h"
#include "util/xml.h"

// The maximum number of effect parameters we're going to support.
const unsigned int kDefaultMaxParameters = 16;

EffectSlot::EffectSlot(const QString& group,
                       EffectsManager* pEffectsManager,
                       const unsigned int iEffectnumber,
                       EngineEffectChain* pEngineEffectChain)
        : m_iEffectNumber(iEffectnumber),
          m_group(group),
          m_pEffectsManager(pEffectsManager),
          m_pEngineEffectChain(pEngineEffectChain),
          m_pEngineEffect(nullptr) {
    VERIFY_OR_DEBUG_ASSERT(m_pEngineEffectChain != nullptr) {
        return;
    }

    m_pControlLoaded = new ControlObject(ConfigKey(m_group, "loaded"));
    m_pControlLoaded->setReadOnly();

    m_pControlNumParameters.insert(EffectManifestParameter::ParameterType::KNOB,
            new ControlObject(ConfigKey(m_group, "num_parameters")));
    m_pControlNumParameters.insert(EffectManifestParameter::ParameterType::BUTTON,
            new ControlObject(ConfigKey(m_group, "num_button_parameters")));
    for (const auto& pControlNumParameters : m_pControlNumParameters) {
        pControlNumParameters->setReadOnly();
    }

    m_pControlNumParameterSlots.insert(EffectManifestParameter::ParameterType::KNOB,
            new ControlObject(ConfigKey(m_group, "num_parameterslots")));
    m_pControlNumParameterSlots.insert(EffectManifestParameter::ParameterType::BUTTON,
            new ControlObject(ConfigKey(m_group, "num_button_parameterslots")));
    for (const auto& pControlNumParameterSlots : m_pControlNumParameterSlots) {
        pControlNumParameterSlots->setReadOnly();
    }

    // Default to disabled to prevent accidental activation of effects
    // at the beginning of a set.
    m_pControlEnabled = new ControlPushButton(ConfigKey(m_group, "enabled"));
    m_pControlEnabled->setButtonMode(ControlPushButton::POWERWINDOW);
    connect(m_pControlEnabled, &ControlObject::valueChanged,
            this, &EffectSlot::updateEngineState);

    m_pControlNextEffect = new ControlPushButton(ConfigKey(m_group, "next_effect"));
    connect(m_pControlNextEffect, &ControlObject::valueChanged,
            this, &EffectSlot::slotNextEffect);

    m_pControlPrevEffect = new ControlPushButton(ConfigKey(m_group, "prev_effect"));
    connect(m_pControlPrevEffect, &ControlObject::valueChanged,
            this, &EffectSlot::slotPrevEffect);

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pControlEffectSelector = new ControlEncoder(ConfigKey(m_group, "effect_selector"), false);
    connect(m_pControlEffectSelector, &ControlObject::valueChanged,
            this, &EffectSlot::slotEffectSelector);

    m_pControlClear = new ControlPushButton(ConfigKey(m_group, "clear"));
    connect(m_pControlClear, &ControlObject::valueChanged,
            this, &EffectSlot::slotClear);

    for (unsigned int i = 0; i < kDefaultMaxParameters; ++i) {
        addEffectParameterSlot(EffectManifestParameter::ParameterType::KNOB);
        addEffectParameterSlot(EffectManifestParameter::ParameterType::BUTTON);
    }

    m_pControlMetaParameter = new ControlPotmeter(ConfigKey(m_group, "meta"), 0.0, 1.0);
    // QObject::connect cannot connect to slots with optional parameters using function
    // pointer syntax if the slot has more parameters than the signal, so use a lambda
    // to hack around this limitation.
    connect(m_pControlMetaParameter, &ControlObject::valueChanged,
            this, [=](double value){slotEffectMetaParameter(value);} );
    m_pControlMetaParameter->set(0.0);
    m_pControlMetaParameter->setDefaultValue(0.0);

    m_pMetaknobSoftTakeover = new SoftTakeover();

    m_pControlLoaded->forceSet(0.0);
}

EffectSlot::~EffectSlot() {
    //qDebug() << debugString() << "destroyed";
    unloadEffect();

    delete m_pControlLoaded;
    for (const auto& pControlNumParameters : m_pControlNumParameters) {
        delete pControlNumParameters;
    }
    for (const auto& pControlNumParameterSlots : m_pControlNumParameterSlots) {
        delete pControlNumParameterSlots;
    }
    delete m_pControlNextEffect;
    delete m_pControlPrevEffect;
    delete m_pControlEffectSelector;
    delete m_pControlClear;
    delete m_pControlEnabled;
    delete m_pControlMetaParameter;
    delete m_pMetaknobSoftTakeover;
}

void EffectSlot::addToEngine(std::unique_ptr<EffectProcessor> pProcessor,
        const QSet<ChannelHandleAndGroup>& activeInputChannels) {
    VERIFY_OR_DEBUG_ASSERT(!isLoaded()) {
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(m_pEngineEffect == nullptr) {
        return;
    }

    m_pEngineEffect = new EngineEffect(m_pManifest,
            activeInputChannels,
            m_pEffectsManager,
            std::move(pProcessor));

    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::ADD_EFFECT_TO_CHAIN;
    request->pTargetChain = m_pEngineEffectChain;
    request->AddEffectToChain.pEffect = m_pEngineEffect;
    request->AddEffectToChain.iIndex = m_iEffectNumber;
    m_pEffectsManager->writeRequest(request);
}

void EffectSlot::removeFromEngine() {
    VERIFY_OR_DEBUG_ASSERT(isLoaded()) {
        return;
    }

    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::REMOVE_EFFECT_FROM_CHAIN;
    request->pTargetChain = m_pEngineEffectChain;
    request->RemoveEffectFromChain.pEffect = m_pEngineEffect;
    request->RemoveEffectFromChain.iIndex = m_iEffectNumber;
    m_pEffectsManager->writeRequest(request);

    m_pEngineEffect = nullptr;
}

void EffectSlot::updateEngineState() {
    if (!m_pEngineEffect) {
        return;
    }

    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::SET_EFFECT_PARAMETERS;
    pRequest->pTargetEffect = m_pEngineEffect;
    pRequest->SetEffectParameters.enabled = m_pControlEnabled->get();
    m_pEffectsManager->writeRequest(pRequest);

    for (auto const& pParameter : m_parameters) {
        pParameter->updateEngineState();
    }
}

EffectState* EffectSlot::createState(const mixxx::EngineParameters& bufferParameters) {
    VERIFY_OR_DEBUG_ASSERT(m_pEngineEffect != nullptr) {
        return new EffectState(bufferParameters);
    }
    return m_pEngineEffect->createState(bufferParameters);
}

EffectManifestPointer EffectSlot::getManifest() const {
    return m_pManifest;
}

void EffectSlot::addEffectParameterSlot(EffectManifestParameter::ParameterType parameterType) {
    EffectParameterSlotBasePointer pParameterSlot = EffectParameterSlotBasePointer();
    if (parameterType == EffectManifestParameter::ParameterType::KNOB) {
        pParameterSlot = static_cast<EffectParameterSlotBasePointer>(
                new EffectKnobParameterSlot(m_group, m_iNumParameterSlots[parameterType]));
    } else if (parameterType == EffectManifestParameter::ParameterType::BUTTON) {
        pParameterSlot = static_cast<EffectParameterSlotBasePointer>(
                new EffectButtonParameterSlot(m_group, m_iNumParameterSlots[parameterType]));
    }
    ++m_iNumParameterSlots[parameterType];
    m_pControlNumParameterSlots[parameterType]->forceSet(
            m_pControlNumParameterSlots[parameterType]->get() + 1);
    VERIFY_OR_DEBUG_ASSERT(m_iNumParameterSlots[parameterType] ==
            m_pControlNumParameterSlots[parameterType]->get()) {
        return;
    }
    m_parameterSlots.append(pParameterSlot);
}

unsigned int EffectSlot::numParameters(EffectManifestParameter::ParameterType parameterType) const {
    unsigned int num = 0;
    for (auto const& pParameter : m_parameters) {
        if (parameterType == pParameter->manifest()->parameterType()) {
            ++num;
        }
    }
    return num;
}

EffectParameter* EffectSlot::getParameterForSlot(EffectManifestParameter::ParameterType parameterType,
                                                 unsigned int slotNumber) {
    // It's normal to ask for a parameter that doesn't exist. Callers must check
    // for NULL.
    unsigned int num = 0;
    for (const auto& parameter: m_parameters) {
        if (parameter->manifest()->showInParameterSlot() && parameter->manifest()->parameterType() == parameterType) {
            if(num == slotNumber) {
                return parameter;
            }
            ++num;
        }
    }
    return nullptr;
}

void EffectSlot::setEnabled(bool enabled) {
    m_pControlEnabled->set(enabled);
}

EffectParameterSlotBasePointer EffectSlot::getEffectParameterSlot(EffectManifestParameter::ParameterType parameterType,
                                                                  unsigned int slotNumber) {
    // qDebug() << debugString() << "getEffectParameterSlot" << static_cast<int>(parameterType) << ' ' << slotNumber;

    int iSlotNumber = 0;
    for (const auto& pParameterSlot : m_parameterSlots) {
        if (pParameterSlot->parameterType() == parameterType) {
            if (iSlotNumber == slotNumber) {
                return pParameterSlot;
            }
            ++iSlotNumber;
        }
    }
    return EffectParameterSlotBasePointer();
}

void EffectSlot::loadEffect(const EffectManifestPointer pManifest,
                            std::unique_ptr<EffectProcessor> pProcessor,
                            const QSet<ChannelHandleAndGroup>& activeChannels) {
    unloadEffect();

    m_pManifest = pManifest;

    if (pManifest == EffectManifestPointer()) {
        // No new effect to load; just unload the old effect and return.
        emit(effectChanged());
        return;
    }

    for (auto& parameterMapping : m_mapForParameterType) {
        parameterMapping.clear();
    }

    addToEngine(std::move(pProcessor), activeChannels);

    int index = 0;
    for (const auto& pManifestParameter: m_pManifest->parameters()) {
        EffectParameter* pParameter = new EffectParameter(
                m_pEngineEffect, m_pEffectsManager, m_parameters.size(), pManifestParameter);
        m_parameters.append(pParameter);

        m_mapForParameterType[pManifestParameter->parameterType()].push_back(index);
        ++index;
    }

    m_pControlLoaded->forceSet(1.0);

    loadParameters();

    if (m_pEffectsManager->isAdoptMetaknobValueEnabled()) {
        slotEffectMetaParameter(m_pControlMetaParameter->get(), true);
    } else {
        m_pControlMetaParameter->set(m_pManifest->metaknobDefault());
        slotEffectMetaParameter(m_pManifest->metaknobDefault(), true);
    }

    emit(effectChanged());
    updateEngineState();
}

void EffectSlot::unloadEffect() {
    if (!isLoaded()) {
        return;
    }

    m_pControlLoaded->forceSet(0.0);
    for (const auto& pControlNumParameters : m_pControlNumParameters) {
        pControlNumParameters->forceSet(0.0);
    }

    for (const auto& pParameterSlot : m_parameterSlots) {
        pParameterSlot->clear();
    }

    for (int i = 0; i < m_parameters.size(); ++i) {
        EffectParameter* pParameter = m_parameters.at(i);
        m_parameters[i] = nullptr;
        delete pParameter;
    }
    m_parameters.clear();
    m_pManifest.clear();
    for (auto& parameterMapping : m_mapForParameterType) {
        parameterMapping.clear();
    }

    removeFromEngine();
}

void EffectSlot::loadParameters() {
    int numTypes = static_cast<int>(EffectManifestParameter::ParameterType::NUM_TYPES);
    for (int parameterTypeId=0 ; parameterTypeId<numTypes ; ++parameterTypeId) {
        const EffectManifestParameter::ParameterType parameterType =
                static_cast<EffectManifestParameter::ParameterType>(parameterTypeId);

        m_pControlNumParameters[parameterType]->forceSet(numParameters(parameterType));

        unsigned int parameterSlotIndex = 0;
        unsigned int numParameterSlots = m_iNumParameterSlots[parameterType];

        // Load EffectParameters into the slot indicated by m_mapForParameterType
        for (int i=0 ; i<m_mapForParameterType[parameterType].size() ; ++i) {
            const int manifestIndex = m_mapForParameterType[parameterType].value(i, -1);

            auto pParameter = m_parameters.value(manifestIndex, nullptr);

            // Try loading the next parameter in the current parameter slot
            VERIFY_OR_DEBUG_ASSERT(pParameter != nullptr) {
                continue;
            }

            VERIFY_OR_DEBUG_ASSERT(parameterSlotIndex < numParameterSlots) {
                break;
            }

            auto pParameterSlot = getEffectParameterSlot(parameterType, parameterSlotIndex);
            VERIFY_OR_DEBUG_ASSERT(pParameterSlot != nullptr) {
                ++parameterSlotIndex;
                continue;
            }
            pParameterSlot->loadParameter(pParameter);
            ++parameterSlotIndex;
        }

        // Clear any EffectParameterSlots that still have a loaded parameter from before
        // but the loop above did not load a new parameter into them
        while (parameterSlotIndex < numParameterSlots) {
            auto pParameterSlot = getEffectParameterSlot(parameterType, parameterSlotIndex);
            VERIFY_OR_DEBUG_ASSERT(pParameterSlot != nullptr) {
                ++parameterSlotIndex;
                continue;
            }
            pParameterSlot->clear();
            ++parameterSlotIndex;
        }
    }
}

void EffectSlot::hideEffectParameter(const unsigned int parameterId) {
    for (auto& parameterMapping : m_mapForParameterType) {
        parameterMapping.removeAll(parameterId);
    }

    loadParameters();
}

void EffectSlot::setEffectParameterPosition(const unsigned int parameterId,
        const unsigned int position) {
    for (auto& parameterMapping : m_mapForParameterType) {
        parameterMapping.removeAll(parameterId);
    }

    auto pParameter = m_parameters.at(parameterId);
    if (pParameter) {
        m_mapForParameterType[pParameter->manifest()->parameterType()].insert(position, parameterId);
        loadParameters();
    }
}

void EffectSlot::slotPrevEffect(double v) {
    if (v > 0) {
        slotEffectSelector(-1);
    }
}

void EffectSlot::slotNextEffect(double v) {
    if (v > 0) {
        slotEffectSelector(1);
    }
}

void EffectSlot::slotEffectSelector(double v) {
    // TODO: reimplement
    // if (v > 0) {
    //     emit(nextEffect(m_iChainNumber, m_iEffectNumber, m_pEffect));
    // } else if (v < 0) {
    //     emit(prevEffect(m_iChainNumber, m_iEffectNumber, m_pEffect));
    // }
}

void EffectSlot::slotClear(double v) {
    if (v > 0) {
        unloadEffect();
        emit(effectChanged());
    }
}

void EffectSlot::syncSofttakeover() {
    for (const auto pParameterSlot : m_parameterSlots) {
        if (pParameterSlot->parameterType() == EffectManifestParameter::ParameterType::KNOB) {
            pParameterSlot->syncSofttakeover();
        }
    }
}

double EffectSlot::getMetaParameter() const {
    return m_pControlMetaParameter->get();
}

// This function is for the superknob to update individual effects' meta knobs
// slotEffectMetaParameter does not need to update m_pControlMetaParameter's value
void EffectSlot::setMetaParameter(double v, bool force) {
    if (!m_pMetaknobSoftTakeover->ignore(m_pControlMetaParameter, v)
            || !m_pControlEnabled->toBool()
            || force) {
        m_pControlMetaParameter->set(v);
        slotEffectMetaParameter(v, force);
    }
}

void EffectSlot::slotEffectMetaParameter(double v, bool force) {
    // Clamp to [0.0, 1.0]
    if (v < 0.0 || v > 1.0) {
        qWarning() << debugString() << "value out of limits";
        v = math_clamp(v, 0.0, 1.0);
        m_pControlMetaParameter->set(v);
    }
    if (!m_pControlEnabled->toBool()) {
        force = true;
    }
    for (const auto& pParameterSlot : m_parameterSlots) {
        if (pParameterSlot->parameterType() == EffectManifestParameter::ParameterType::KNOB) {
            pParameterSlot->onEffectMetaParameterChanged(v, force);
        }
    }
}

QDomElement EffectSlot::toXml(QDomDocument* doc) const {
    QDomElement effectElement = doc->createElement(EffectXml::Effect);
    // if (!m_pEffect) {
        // return effectElement;
    // }

    // QDomElement metaKnobElement = doc->createElement(EffectXml::EffectMetaParameter);
    // XmlParse::addElement(*doc, effectElement,
    //                      EffectXml::EffectMetaParameter,
    //                      QString::number(m_pControlMetaParameter->get()));
    // EffectManifestPointer pManifest = m_pEffect->getManifest();
    // XmlParse::addElement(*doc, effectElement,
    //                      EffectXml::EffectId, pManifest->id());
    // XmlParse::addElement(*doc, effectElement,
    //                      EffectXml::EffectVersion, pManifest->version());

    // QDomElement parametersElement = doc->createElement(EffectXml::ParametersRoot);

    // for (const auto& pParameter : m_knobParameterSlots) {
    //     QDomElement parameterElement = pParameter->toXml(doc);
    //     if (!parameterElement.hasChildNodes()) {
    //         continue;
    //     }
    //     EffectManifestParameterPointer manifest = pParameter->getManifest();
    //     if (!manifest) {
    //         continue;
    //     }
    //     XmlParse::addElement(*doc, parameterElement,
    //                          EffectXml::ParameterId,
    //                          manifest->id());
    //     parametersElement.appendChild(parameterElement);
    // }
    // for (const auto& pParameter : m_buttonParameterSlots) {
    //     QDomElement parameterElement = pParameter->toXml(doc);
    //     if (!parameterElement.hasChildNodes()) {
    //         continue;
    //     }
    //     EffectManifestParameterPointer manifest = pParameter->getManifest();
    //     if (!manifest) {
    //         continue;
    //     }
    //     XmlParse::addElement(*doc, parameterElement,
    //                          EffectXml::ParameterId,
    //                          pParameter->getManifest()->id());
    //     parametersElement.appendChild(parameterElement);
    // }

    // effectElement.appendChild(parametersElement);

    return effectElement;
}

void EffectSlot::loadEffectSlotFromXml(const QDomElement& effectElement) {
    // if (!m_pEffect) {
    //     return;
    // }

    // if (!effectElement.hasChildNodes()) {
    //     return;
    // }

    // QDomElement effectIdElement = XmlParse::selectElement(effectElement,
    //                                                       EffectXml::EffectId);
    // if (m_pEffect->getManifest()->id() != effectIdElement.text()) {
    //     qWarning() << "EffectSlot::loadEffectSlotFromXml"
    //                << "effect ID in XML does not match presently loaded effect, ignoring.";
    //     return;
    // }

    // m_pControlMetaParameter->set(XmlParse::selectNodeDouble(
    //         effectElement, EffectXml::EffectMetaParameter));
    // QDomElement parametersElement = XmlParse::selectElement(
    //         effectElement, EffectXml::ParametersRoot);
    // if (!parametersElement.hasChildNodes()) {
    //     return;
    // }

    // QMap<QString, EffectParameterSlotBasePointer> parametersById;
    // for (const auto& pParameter : m_knobParameterSlots) {
    //     EffectManifestParameterPointer manifest = pParameter->getManifest();
    //     if (manifest) {
    //         parametersById.insert(manifest->id(), pParameter);
    //     }
    // }
    // for (const auto& pParameter : m_buttonParameterSlots) {
    //     EffectManifestParameterPointer manifest = pParameter->getManifest();
    //     if (manifest) {
    //         parametersById.insert(manifest->id(), pParameter);
    //     }
    // }

    // QDomNodeList parametersNodeList = parametersElement.childNodes();
    // for (int i = 0; i < parametersNodeList.size(); ++i) {
    //     QDomNode parameterNode = parametersNodeList.at(i);
    //     if (parameterNode.isElement()) {
    //         const QString id = XmlParse::selectNodeQString(parameterNode,
    //                                                        EffectXml::ParameterId);
    //         EffectParameterSlotBasePointer pParameterSlot = parametersById.value(id);
    //         if (pParameterSlot != nullptr) {
    //             pParameterSlot->loadParameterSlotFromXml(parameterNode.toElement());
    //         }
    //     }
    // }
}
