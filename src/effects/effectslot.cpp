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

    for (const auto& parameterList : m_parameters) {
        for (auto const& pParameter : parameterList) {
            pParameter->updateEngineState();
        }
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
    m_parameterSlots[parameterType].append(pParameterSlot);
}

unsigned int EffectSlot::numParameters(EffectManifestParameter::ParameterType parameterType) const {
    return m_parameters.value(parameterType).size();
}

void EffectSlot::setEnabled(bool enabled) {
    m_pControlEnabled->set(enabled);
}

EffectParameterSlotBasePointer EffectSlot::getEffectParameterSlot(
        EffectManifestParameter::ParameterType parameterType,
        unsigned int slotNumber) {
    VERIFY_OR_DEBUG_ASSERT(slotNumber <= (unsigned)m_parameterSlots.value(parameterType).size()) {
        return nullptr;
    }
    return m_parameterSlots.value(parameterType).at(slotNumber);
}

void EffectSlot::loadEffect(const EffectManifestPointer pManifest,
        std::unique_ptr<EffectProcessor> pProcessor,
        EffectPresetPointer pEffectPreset,
        const QSet<ChannelHandleAndGroup>& activeChannels) {
    if (kEffectDebugOutput) {
        if (pManifest != nullptr) {
            qDebug() << this << m_group << "loading effect" << pManifest->id() << pEffectPreset.get() << pProcessor.get();
        } else {
            qDebug() << this << m_group << "unloading effect";
        }
    }
    unloadEffect();

    m_pManifest = pManifest;

    if (pManifest == EffectManifestPointer()) {
        // No new effect to load; just unload the old effect and return.
        emit effectChanged();
        return;
    }

    addToEngine(std::move(pProcessor), activeChannels);

    // Create EffectParameters. Every parameter listed in the manifest must have
    // an EffectParameter created, regardless of whether it is loaded in a slot.
    int manifestIndex = 0;
    for (const auto& pManifestParameter: m_pManifest->parameters()) {
        // match the manifest parameter to the preset parameter
        EffectParameterPreset parameterPreset = EffectParameterPreset();
        if (pEffectPreset != nullptr) {
            for (const auto& p : pEffectPreset->getParameterPresets()) {
                if (p.id() == pManifestParameter->id()) {
                    parameterPreset = p;
                }
            }
        }
        EffectParameterPointer pParameter(new EffectParameter(
                m_pEngineEffect, m_pEffectsManager, manifestIndex, pManifestParameter, parameterPreset));
        m_parameters[pManifestParameter->parameterType()].append(pParameter);
        manifestIndex++;
    }

    // Map the parameter slots to the EffectParameters.
    // The slot order is determined by the order parameters are listed in the preset.
    int numTypes = static_cast<int>(EffectManifestParameter::ParameterType::NUM_TYPES);
    for (int parameterTypeId = 0; parameterTypeId < numTypes; ++parameterTypeId) {
        const EffectManifestParameter::ParameterType parameterType =
                static_cast<EffectManifestParameter::ParameterType>(parameterTypeId);

        if (pEffectPreset != nullptr && !pEffectPreset.isNull()) {
            m_loadedParameters[parameterType].clear();
            int slot = 0;
            for (const auto& parameterPreset : pEffectPreset->getParameterPresets()) {
                if (parameterPreset.hidden() || parameterPreset.isNull()) {
                    continue;
                }

                for (const auto& pParameter : m_parameters.value(parameterType)) {
                    if (pParameter->manifest()->id() == parameterPreset.id()) {
                        m_loadedParameters[parameterType].insert(slot, pParameter);
                        break;
                    }
                }
                slot++;
            }
        }
    }

    loadParameters();

    m_pControlLoaded->forceSet(1.0);

    // TODO: load meta knob from preset
    if (m_pEffectsManager->isAdoptMetaknobValueEnabled()) {
        slotEffectMetaParameter(m_pControlMetaParameter->get(), true);
    } else {
        m_pControlMetaParameter->set(m_pManifest->metaknobDefault());
        slotEffectMetaParameter(m_pManifest->metaknobDefault(), true);
    }

    emit effectChanged();
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

    for (auto& slotList : m_parameterSlots) {
        // Do not delete the slots; clear the parameters from the slots
        // The parameter slots are used by the next effect, but the EffectParameters
        // are deleted below.
        for (auto pSlot : slotList) {
            pSlot->clear();
        }
    }
    for (auto& parameterList : m_parameters) {
        parameterList.clear();
    }
    for (auto& parameterList : m_loadedParameters) {
        parameterList.clear();
    }

    m_pManifest.clear();

    removeFromEngine();
}

void EffectSlot::loadParameters() {
    //qDebug() << this << m_group << "loading parameters";
    int numTypes = static_cast<int>(EffectManifestParameter::ParameterType::NUM_TYPES);
    for (int parameterTypeId=0 ; parameterTypeId<numTypes ; ++parameterTypeId) {
        const EffectManifestParameter::ParameterType parameterType =
                static_cast<EffectManifestParameter::ParameterType>(parameterTypeId);

        m_pControlNumParameters[parameterType]->forceSet(numParameters(parameterType));

        int slot = 0;
        for (auto pParameter : m_loadedParameters.value(parameterType)) {
            VERIFY_OR_DEBUG_ASSERT(slot <= m_parameterSlots.value(parameterType).size()) {
                break;
            }
            m_parameterSlots.value(parameterType).at(slot)->loadParameter(pParameter);
            slot++;
        }

        // Clear any EffectParameterSlots that still have a loaded parameter from before
        // but the loop above did not load a new parameter into them.
        for (; slot < m_parameterSlots.value(parameterType).size(); slot++) {
            m_parameterSlots.value(parameterType).at(slot)->clear();
        }
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
    //     emit nextEffect(m_iChainNumber, m_iEffectNumber, m_pEffect);
    // } else if (v < 0) {
    //     emit prevEffect(m_iChainNumber, m_iEffectNumber, m_pEffect);
    // }
}

void EffectSlot::slotClear(double v) {
    if (v > 0) {
        unloadEffect();
        emit effectChanged();
    }
}

void EffectSlot::syncSofttakeover() {
    for (const auto& parameterSlotList : m_parameterSlots) {
        for (const auto& pParameterSlot : parameterSlotList) {
            if (pParameterSlot->parameterType() == EffectManifestParameter::ParameterType::KNOB) {
                pParameterSlot->syncSofttakeover();
            }
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

    // Only knobs are linked to the metaknob; not buttons
    for (const auto& pParameterSlot : m_parameterSlots.value(EffectManifestParameter::ParameterType::KNOB)) {
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
