#include "effects/presets/effectparameterpreset.h"

#include "effects/effectparameter.h"
#include "effects/presets/effectxmlelements.h"
#include "util/xml.h"

EffectParameterPreset::EffectParameterPreset() {
}

EffectParameterPreset::EffectParameterPreset(const QDomElement& parameterElement) {
    VERIFY_OR_DEBUG_ASSERT(parameterElement.tagName() == EffectXml::kParameter) {
        return;
    }
    if (!parameterElement.hasChildNodes()) {
        m_dValue = 0.0;
        m_id = QString();
        m_linkType = EffectManifestParameter::LinkType::None;
    } else {
        m_id = XmlParse::selectNodeQString(parameterElement, EffectXml::kParameterId);

        m_dValue = XmlParse::selectNodeDouble(parameterElement,
                EffectXml::kParameterValue);

        m_linkType = EffectManifestParameter::LinkTypeFromString(
                XmlParse::selectNodeQString(parameterElement,
                        EffectXml::kParameterLinkType));

        m_linkInversion = static_cast<EffectManifestParameter::LinkInversion>(
                static_cast<int>(XmlParse::selectNodeDouble(parameterElement,
                        EffectXml::kParameterLinkInversion)));

        m_bHidden = XmlParse::selectNodeBool(parameterElement,
                EffectXml::kParameterHidden);
    }
}

EffectParameterPreset::EffectParameterPreset(const EffectParameterPointer pParameter, bool hidden) {
    m_id = pParameter->manifest()->id();
    m_dValue = pParameter->getValue();
    m_linkType = pParameter->linkType();
    m_linkInversion = pParameter->linkInversion();
    m_bHidden = hidden;
}

EffectParameterPreset::EffectParameterPreset(
        const EffectManifestParameterPointer pManifestParameter) {
    m_id = pManifestParameter->id();
    m_dValue = pManifestParameter->getDefault();
    m_linkType = pManifestParameter->defaultLinkType();
    m_linkInversion = pManifestParameter->defaultLinkInversion();
    m_bHidden = false;
}

const QDomElement EffectParameterPreset::toXml(QDomDocument* doc) const {
    QDomElement parameterElement;
    if (!m_id.isEmpty()) {
        parameterElement = doc->createElement(EffectXml::kParameter);
        XmlParse::addElement(*doc,
                parameterElement,
                EffectXml::kEffectId,
                m_id);
        XmlParse::addElement(*doc,
                parameterElement,
                EffectXml::kParameterValue,
                QString::number(m_dValue));
        XmlParse::addElement(*doc,
                parameterElement,
                EffectXml::kParameterLinkType,
                EffectManifestParameter::LinkTypeToString(m_linkType));
        XmlParse::addElement(*doc,
                parameterElement,
                EffectXml::kParameterLinkInversion,
                QString::number(static_cast<int>(m_linkInversion)));
        XmlParse::addElement(*doc,
                parameterElement,
                EffectXml::kParameterHidden,
                QString::number(static_cast<int>(m_bHidden)));
    }
    return parameterElement;
}

EffectParameterPreset::~EffectParameterPreset() {
}
