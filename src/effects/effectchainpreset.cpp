#include "effects/effectchainpreset.h"

#include "effects/effectxmlelements.h"
#include "effects/effectchainslot.h"
#include "util/xml.h"

EffectChainPreset::EffectChainPreset() {
}

EffectChainPreset::EffectChainPreset(const QDomElement& element) {
    if (!element.hasChildNodes()) {
        return;
    }

    m_id = XmlParse::selectNodeQString(element, EffectXml::ChainId);
    m_name = XmlParse::selectNodeQString(element, EffectXml::ChainName);
    m_description = XmlParse::selectNodeQString(element, EffectXml::ChainDescription);

    QString mixModeStr = XmlParse::selectNodeQString(element, EffectXml::ChainMixMode);
    m_mixMode = EffectChainSlot::mixModeFromString(mixModeStr);

    m_dSuper = XmlParse::selectNodeDouble(element, EffectXml::ChainSuperParameter);

    QDomElement effectsElement = XmlParse::selectElement(element, EffectXml::EffectsRoot);
    QDomNodeList effectList = effectsElement.childNodes();

    for (int i = 0; i < effectList.count(); ++i) {
        QDomNode effectNode = effectList.at(i);
        if (effectNode.isElement()) {
            QDomElement effectElement = effectNode.toElement();
            EffectPresetPointer pPreset(new EffectPreset(effectElement));
            m_effectPresets.append(pPreset);
        }
    }
}

EffectChainPreset::~EffectChainPreset() {
}
