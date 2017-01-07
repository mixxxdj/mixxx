namespace EffectXml {
// Version history:
// 0 (Mixxx 2.1.0): initial support for saving state of effects
const int kXmlSchemaVersion = 0;

const QString Root("MixxxEffects");
const QString SchemaVersion("SchemaVersion");

const QString Rack("Rack");
const QString RackGroup("Group");

const QString ChainsRoot("Chains");
const QString Chain("EffectChain");
const QString ChainSuperParameter("SuperParameterValue");
const QString ChainId("Id");
const QString ChainName("Name");
const QString ChainDescription("Description");
const QString ChainInsertionType("InsertionType");

const QString EffectsRoot("Effects");
const QString Effect("Effect");
const QString EffectMetaParameter("MetaParameterValue");
const QString EffectId("Id");
const QString EffectVersion("Version");

const QString ParametersRoot("Parameters");
const QString Parameter("Parameter");

const QString KnobParametersRoot("KnobParameters");
const QString KnobParameter("KnobParameter");
const QString KnobParameterId("Id");
const QString KnobParameterValue("Value");
const QString KnobParameterLinkType("LinkType");
const QString KnobParameterLinkInversion("LinkInversion");

const QString ButtonParametersRoot("ButtonParameters");
const QString ButtonParameter("ButtonParameter");
const QString ButtonParameterId("Id");
const QString ButtonParameterValue("Value");
}
