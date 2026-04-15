#pragma once

namespace EffectXml {
const QString kFileHeader(QStringLiteral("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"));
// Version history:
// 0 (Mixxx 2.1.0): initial support for saving state of effects
constexpr int kXmlSchemaVersion = 0;

const QString kRoot(QStringLiteral("MixxxEffects"));
const QString kSchemaVersion(QStringLiteral("SchemaVersion"));

const QString kRack(QStringLiteral("Rack"));
const QString kMainEq(QStringLiteral("MainEQ"));
const QString kRackGroup(QStringLiteral("Group"));

const QString kChainPresetList(QStringLiteral("ChainPresetList"));
const QString kQuickEffectList(QStringLiteral("QuickEffectPresetList"));
const QString kStemQuickEffectChainPresets(QStringLiteral("StemQuickEffectChains"));
const QString kQuickEffectChainPresets(QStringLiteral("QuickEffectChains"));
const QString kEqualizerEffects(QStringLiteral("EqualizerEffects"));
const QString kChainPresetName(QStringLiteral("ChainPresetName"));

const QString kVisibleEffects(QStringLiteral("VisibleEffects"));
const QString kHiddenEffects(QStringLiteral("HiddenEffects"));

const QString kChainsRoot(QStringLiteral("Chains"));
const QString kChain(QStringLiteral("EffectChain"));
const QString kChainSuperParameter(QStringLiteral("SuperParameterValue"));
const QString kChainId(QStringLiteral("Id"));
const QString kChainName(QStringLiteral("Name"));
const QString kChainDescription(QStringLiteral("Description"));
const QString kChainMixMode(QStringLiteral("MixMode"));

const QString kEffectsRoot(QStringLiteral("Effects"));
const QString kEffect(QStringLiteral("Effect"));
const QString kEffectMetaParameter(QStringLiteral("MetaParameterValue"));
const QString kEffectId(QStringLiteral("Id"));
const QString kEffectBackendType(QStringLiteral("BackendType"));

const QString kParametersRoot(QStringLiteral("Parameters"));
const QString kParameter(QStringLiteral("Parameter"));
const QString kParameterId(QStringLiteral("Id"));
const QString kParameterValue(QStringLiteral("Value"));
const QString kParameterLinkType(QStringLiteral("LinkType"));
const QString kParameterLinkInversion(QStringLiteral("LinkInversion"));
const QString kParameterHidden(QStringLiteral("Hidden"));
} // namespace EffectXml
