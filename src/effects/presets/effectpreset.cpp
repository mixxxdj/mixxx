#include "effects/presets/effectpreset.h"

#include "effects/backends/effectsbackend.h"
#include "effects/effectslot.h"
#include "effects/presets/effectxmlelements.h"
#include "util/xml.h"

EffectPreset::EffectPreset()
        : m_backendType(EffectBackendType::Unknown),
          m_dMetaParameter(0.0) {
}

EffectPreset::EffectPreset(const QDomElement& effectElement)
        : EffectPreset() {
    // effectElement can come from untrusted input from the filesystem, so do not DEBUG_ASSERT
    if (effectElement.tagName() != EffectXml::kEffect) {
        return;
    }
    if (!effectElement.hasChildNodes()) {
        return;
    }

    m_id = XmlParse::selectNodeQString(effectElement, EffectXml::kEffectId);
    m_backendType = EffectsBackend::backendTypeFromString(
            XmlParse::selectNodeQString(effectElement,
                    EffectXml::kEffectBackendType));
    m_dMetaParameter = XmlParse::selectNodeDouble(effectElement, EffectXml::kEffectMetaParameter);

    QDomElement parametersElement =
            XmlParse::selectElement(effectElement, EffectXml::kParametersRoot);
    QDomNodeList parametersList = parametersElement.childNodes();

    for (int i = 0; i < parametersList.count(); ++i) {
        QDomNode parameterNode = parametersList.at(i);
        if (parameterNode.isElement()) {
            QDomElement parameterElement = parameterNode.toElement();
            EffectParameterPreset parameterPreset(parameterElement);
            m_effectParameterPresets.append(parameterElement);
        }
    }
}

EffectPreset::EffectPreset(const EffectSlotPointer pEffectSlot)
        : m_id(pEffectSlot->id()),
          m_backendType(pEffectSlot->backendType()),
          m_dMetaParameter(pEffectSlot->getMetaParameter()) {
    // Parameters are reloaded in the order they are saved, so the order of
    // loaded effects must be preserved.
    int numTypes = static_cast<int>(EffectParameterType::NumTypes);
    for (int parameterTypeId = 0; parameterTypeId < numTypes; ++parameterTypeId) {
        const EffectParameterType parameterType =
                static_cast<EffectParameterType>(parameterTypeId);
        for (const auto& pParameter : pEffectSlot->getLoadedParameters().value(parameterType)) {
            m_effectParameterPresets.append(EffectParameterPreset(pParameter, false));
        }
        for (const auto& pParameter : pEffectSlot->getHiddenParameters().value(parameterType)) {
            m_effectParameterPresets.append(EffectParameterPreset(pParameter, true));
        }
    }
}

EffectPreset::EffectPreset(const EffectManifestPointer pManifest)
        : m_id(pManifest->id()),
          m_backendType(pManifest->backendType()),
          m_dMetaParameter(pManifest->metaknobDefault()) {
    for (const auto& pParameterManifest : pManifest->parameters()) {
        m_effectParameterPresets.append(EffectParameterPreset(pParameterManifest));
    }
}

const QDomElement EffectPreset::toXml(QDomDocument* doc) const {
    QDomElement effectElement = doc->createElement(EffectXml::kEffect);
    if (m_id.isEmpty()) {
        return effectElement;
    }

    XmlParse::addElement(
            *doc,
            effectElement,
            EffectXml::kEffectMetaParameter,
            QString::number(m_dMetaParameter));
    XmlParse::addElement(
            *doc,
            effectElement,
            EffectXml::kEffectId,
            m_id);
    XmlParse::addElement(
            *doc,
            effectElement,
            EffectXml::kEffectBackendType,
            EffectsBackend::backendTypeToString(m_backendType));

    QDomElement parametersElement = doc->createElement(EffectXml::kParametersRoot);
    for (const auto& pParameter : std::as_const(m_effectParameterPresets)) {
        parametersElement.appendChild(pParameter.toXml(doc));
    }
    effectElement.appendChild(parametersElement);

    return effectElement;
}

void EffectPreset::updateParametersFrom(const EffectPreset& other) {
    DEBUG_ASSERT(backendType() == other.backendType());

    // technically algorithmically inefficient solution O(n²). May be
    // optimizable by sorting first, gains depend on parameter count
    for (const auto& parameterToCopy : other.m_effectParameterPresets) {
        auto currentParameterIt =
                std::find_if(m_effectParameterPresets.begin(),
                        m_effectParameterPresets.end(),
                        [&](const auto& ourParameter) {
                            return ourParameter.id() == parameterToCopy.id();
                        });
        if (currentParameterIt == m_effectParameterPresets.end()) {
            continue;
        }
        // overwrite our parameter by taking a copy of the same parameter from `other`
        *currentParameterIt = parameterToCopy;
    }
}
