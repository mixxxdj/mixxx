#include "effects/presets/effectchainpreset.h"

#include "effects/backends/effectmanifest.h"
#include "effects/effectchain.h"
#include "effects/presets/effectpreset.h"
#include "effects/presets/effectxmlelements.h"
#include "util/xml.h"

EffectChainPreset::EffectChainPreset()
        : m_name(kNoEffectString),
          m_mixMode(EffectChainMixMode::DrySlashWet),
          m_dSuper(0.0),
          m_readOnly(false) {
}

EffectChainPreset::EffectChainPreset(const QDomElement& chainElement) {
    // chainElement can come from untrusted input from the filesystem, so do not DEBUG_ASSERT
    if (chainElement.tagName() != EffectXml::kChain) {
        qWarning() << "Effect chain preset has no" << EffectXml::kChain << "XML element";
        return;
    }
    if (!chainElement.hasChildNodes()) {
        qWarning() << "Effect chain preset has no child XML nodes";
        return;
    }

    m_name = XmlParse::selectNodeQString(chainElement, EffectXml::kChainName);

    QString mixModeStr = XmlParse::selectNodeQString(chainElement, EffectXml::kChainMixMode);
    m_mixMode = EffectChainMixMode::fromString(mixModeStr);

    m_dSuper = XmlParse::selectNodeDouble(chainElement, EffectXml::kChainSuperParameter);

    m_readOnly = false;

    QDomElement effectsElement = XmlParse::selectElement(chainElement, EffectXml::kEffectsRoot);
    QDomNodeList effectList = effectsElement.childNodes();

    for (int i = 0; i < effectList.count(); ++i) {
        QDomNode effectNode = effectList.at(i);
        if (effectNode.isElement()) {
            QDomElement effectElement = effectNode.toElement();
            auto pPreset = EffectPresetPointer::create(effectElement);
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
          m_dSuper(chain->getSuperParameter()),
          m_readOnly(false) {
    for (const auto& pEffectSlot : chain->getEffectSlots()) {
        m_effectPresets.append(EffectPresetPointer::create(pEffectSlot));
    }
}

EffectChainPreset::EffectChainPreset(EffectManifestPointer pManifest)
        : m_name(pManifest->displayName()),
          m_mixMode(EffectChainMixMode::DrySlashWet),
          m_dSuper(pManifest->metaknobDefault()),
          m_readOnly(false) {
    m_effectPresets.append(EffectPresetPointer::create(pManifest));
}

EffectChainPreset::EffectChainPreset(EffectPresetPointer pEffectPreset)
        : m_name(pEffectPreset->id()),
          m_mixMode(EffectChainMixMode::DrySlashWet),
          m_dSuper(pEffectPreset->metaParameter()),
          m_readOnly(false) {
    m_effectPresets.append(pEffectPreset);
}

const QDomElement EffectChainPreset::toXml(QDomDocument* doc) const {
    QDomElement chainElement = doc->createElement(EffectXml::kChain);

    XmlParse::addElement(*doc,
            chainElement,
            EffectXml::kChainName,
            m_name);
    XmlParse::addElement(*doc,
            chainElement,
            EffectXml::kChainMixMode,
            EffectChainMixMode::toString(m_mixMode));
    XmlParse::addElement(*doc,
            chainElement,
            EffectXml::kChainSuperParameter,
            QString::number(m_dSuper));

    QDomElement effectsElement = doc->createElement(EffectXml::kEffectsRoot);
    for (const auto& pEffectPreset : m_effectPresets) {
        effectsElement.appendChild(pEffectPreset->toXml(doc));
    }
    chainElement.appendChild(effectsElement);

    return chainElement;
}

EffectChainPreset::~EffectChainPreset() {
}
