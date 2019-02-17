#include "effects/effectpreset.h"

#include "effects/effectxmlelements.h"
#include "util/xml.h"

EffectPreset::EffectPreset() {
}

EffectPreset::EffectPreset(const QDomElement& element) {
    if (!element.hasChildNodes()) {
        return;
    }

    m_id = XmlParse::selectNodeQString(element, EffectXml::EffectId);
    m_version = XmlParse::selectNodeQString(element, EffectXml::EffectVersion);
    m_dMetaParameter = XmlParse::selectNodeDouble(element, EffectXml::EffectMetaParameter);

    QDomElement parametersElement = XmlParse::selectElement(element, EffectXml::ParametersRoot);
    QDomNodeList parametersList = parametersElement.childNodes();

    for (int i = 0; i < parametersList.count(); ++i) {
        QDomNode parameterNode = parametersList.at(i);
        if (parameterNode.isElement()) {
            QDomElement parameterElement = parameterNode.toElement();
            // EffectParameterPresetPointer pPreset(new EffectParameterPreset(parameterElement));
            // m_effectParameterPresets.append(pPreset);
        }
    }
}

EffectPreset::~EffectPreset() {
}
