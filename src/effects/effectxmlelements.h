namespace EffectXml {
const QString FileHeader(QStringLiteral("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"));
// Version history:
// 0 (Mixxx 2.1.0): initial support for saving state of effects
const int kXmlSchemaVersion = 0;

const QString Root(QStringLiteral("MixxxEffects"));
const QString SchemaVersion(QStringLiteral("SchemaVersion"));

const QString Rack(QStringLiteral("Rack"));
const QString RackGroup(QStringLiteral("Group"));

const QString ChainPresetList(QStringLiteral("ChainPresetList"));
const QString QuickEffectList(QStringLiteral("QuickEffectPresetList"));
const QString QuickEffectChainPresets(QStringLiteral("QuickEffectChains"));
const QString ChainPresetName(QStringLiteral("ChainPresetName"));

const QString VisibleEffects(QStringLiteral("VisibleEffects"));

const QString ChainsRoot(QStringLiteral("Chains"));
const QString Chain(QStringLiteral("EffectChain"));
const QString ChainSuperParameter(QStringLiteral("SuperParameterValue"));
const QString ChainId(QStringLiteral("Id"));
const QString ChainName(QStringLiteral("Name"));
const QString ChainDescription(QStringLiteral("Description"));
const QString ChainMixMode(QStringLiteral("MixMode"));

const QString EffectsRoot(QStringLiteral("Effects"));
const QString Effect(QStringLiteral("Effect"));
const QString EffectMetaParameter(QStringLiteral("MetaParameterValue"));
const QString EffectId(QStringLiteral("Id"));
const QString EffectBackendType(QStringLiteral("BackendType"));

const QString ParametersRoot(QStringLiteral("Parameters"));
const QString Parameter(QStringLiteral("Parameter"));
const QString ParameterId(QStringLiteral("Id"));
const QString ParameterValue(QStringLiteral("Value"));
const QString ParameterLinkType(QStringLiteral("LinkType"));
const QString ParameterLinkInversion(QStringLiteral("LinkInversion"));
const QString ParameterHidden(QStringLiteral("Hidden"));
} // namespace EffectXml
