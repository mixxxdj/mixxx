#include "effects/backends/effectmanifestparameter.h"

QDebug operator<<(QDebug dbg, const EffectManifestParameter& parameter) {
    return dbg.maybeSpace() << QString("EffectManifestParameter(%1)").arg(parameter.id());
}
