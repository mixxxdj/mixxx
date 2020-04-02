#include "effects/presets/effectpreset.h"

#include "effects/effectxmlelements.h"
#include "util/xml.h"

EffectPreset::EffectPreset() {
}

EffectPreset::EffectPreset(const QDomElement& effectElement) {
    if (!effectElement.hasChildNodes()) {
        return;
    }

    m_id = XmlParse::selectNodeQString(effectElement, EffectXml::EffectId);
    m_version = XmlParse::selectNodeQString(effectElement, EffectXml::EffectVersion);
    m_dMetaParameter = XmlParse::selectNodeDouble(effectElement, EffectXml::EffectMetaParameter);

    QDomElement parametersElement = XmlParse::selectElement(effectElement, EffectXml::ParametersRoot);
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

EffectPreset::~EffectPreset() {
}
