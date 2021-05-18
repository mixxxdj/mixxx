#include "effects/presets/effectchainpreset.h"

#include "effects/effectchain.h"
#include "effects/presets/effectxmlelements.h"
#include "util/xml.h"

EffectChainPreset::EffectChainPreset()
        : m_name(kNoEffectString),
          m_mixMode(EffectChainMixMode::DrySlashWet),
          m_dSuper(0.0) {
}

EffectChainPreset::EffectChainPreset(const QDomElement& chainElement) {
    // chainElement can come from untrusted input from the filesystem, so do not DEBUG_ASSERT
    if (chainElement.tagName() != EffectXml::Chain) {
        qWarning() << "Effect chain preset has no" << EffectXml::Chain << "XML element";
        return;
    }
    if (!chainElement.hasChildNodes()) {
        qWarning() << "Effect chain preset has no child XML nodes";
        return;
    }

    m_name = XmlParse::selectNodeQString(chainElement, EffectXml::ChainName);

    QString mixModeStr = XmlParse::selectNodeQString(chainElement, EffectXml::ChainMixMode);
    m_mixMode = EffectChainMixMode::fromString(mixModeStr);

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
    if (isEmpty()) {
        qWarning() << "Effect chain preset has no effects";
    }
}

EffectChainPreset::EffectChainPreset(const EffectChain* chain)
        : m_name(chain->presetName()),
          m_mixMode(chain->mixMode()),
          m_dSuper(chain->getSuperParameter()) {
    for (const auto& pEffectSlot : chain->getEffectSlots()) {
        m_effectPresets.append(EffectPresetPointer(new EffectPreset(pEffectSlot)));
    }
}

EffectChainPreset::EffectChainPreset(EffectManifestPointer pManifest)
        : m_name(pManifest->displayName()),
          m_mixMode(EffectChainMixMode::DrySlashWet),
          m_dSuper(pManifest->metaknobDefault()) {
    m_effectPresets.append(EffectPresetPointer(new EffectPreset(pManifest)));
}

EffectChainPreset::EffectChainPreset(EffectPresetPointer pEffectPreset)
        : m_name(pEffectPreset->id()),
          m_mixMode(EffectChainMixMode::DrySlashWet),
          m_dSuper(pEffectPreset->metaParameter()) {
    m_effectPresets.append(pEffectPreset);
}

const QDomElement EffectChainPreset::toXml(QDomDocument* doc) const {
    QDomElement chainElement = doc->createElement(EffectXml::Chain);

    XmlParse::addElement(*doc,
            chainElement,
            EffectXml::ChainName,
            m_name);
    XmlParse::addElement(*doc,
            chainElement,
            EffectXml::ChainMixMode,
            EffectChainMixMode::toString(m_mixMode));
    XmlParse::addElement(*doc,
            chainElement,
            EffectXml::ChainSuperParameter,
            QString::number(m_dSuper));

    QDomElement effectsElement = doc->createElement(EffectXml::EffectsRoot);
    for (const auto& pEffectPreset : m_effectPresets) {
        effectsElement.appendChild(pEffectPreset->toXml(doc));
    }
    chainElement.appendChild(effectsElement);

    return chainElement;
}

EffectChainPreset::~EffectChainPreset() {
}
