#include "effects/backends/au/aumanifest.h"

#include "effects/defs.h"

AUManifest::AUManifest(const QString& id, const QString& name) {
    setId(id);
    setName(name);
    setBackendType(EffectBackendType::AU);
}
