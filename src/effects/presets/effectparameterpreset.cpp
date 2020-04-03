#include "effects/presets/effectparameterpreset.h"

#include "effects/effectxmlelements.h"
#include "util/xml.h"

EffectParameterPreset::EffectParameterPreset() {
}

EffectParameterPreset::EffectParameterPreset(const QDomElement& parameterElement) {
    if (!parameterElement.hasChildNodes()) {
        m_dValue = 0.0;
        m_id = QString();
        m_linkType = EffectManifestParameter::LinkType::NONE;
    } else {
        bool conversionWorked = false;
        m_dValue = XmlParse::selectNodeDouble(parameterElement,
                EffectXml::ParameterValue,
                &conversionWorked);

        m_linkType = EffectManifestParameter::LinkTypeFromString(
                XmlParse::selectNodeQString(parameterElement,
                        EffectXml::ParameterLinkType));

        m_linkInversion = static_cast<EffectManifestParameter::LinkInversion>(
                static_cast<int>(XmlParse::selectNodeDouble(parameterElement,
                        EffectXml::ParameterLinkInversion)));

        m_bHidden = XmlParse::selectNodeBool(parameterElement,
                EffectXml::ParameterHidden);
    }
}

EffectParameterPreset::~EffectParameterPreset() {
}
