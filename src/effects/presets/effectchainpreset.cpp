#include "effects/presets/effectchainpreset.h"

#include "effects/effectchainslot.h"
#include "effects/effectxmlelements.h"
#include "util/xml.h"

EffectChainPreset::EffectChainPreset() {
}

EffectChainPreset::EffectChainPreset(const QDomElement& chainElement) {
    if (!chainElement.hasChildNodes()) {
        return;
    }

    m_id = XmlParse::selectNodeQString(chainElement, EffectXml::ChainId);
    m_name = XmlParse::selectNodeQString(chainElement, EffectXml::ChainName);
    m_description = XmlParse::selectNodeQString(chainElement, EffectXml::ChainDescription);

    QString mixModeStr = XmlParse::selectNodeQString(chainElement, EffectXml::ChainMixMode);
    m_mixMode = EffectChainSlot::mixModeFromString(mixModeStr);

    m_dSuper = XmlParse::selectNodeDouble(chainElement, EffectXml::ChainSuperParameter);

    QDomElement effectsElement = XmlParse::selectElement(chainElement, EffectXml::EffectsRoot);
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
