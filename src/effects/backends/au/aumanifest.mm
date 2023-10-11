#include "effects/backends/au/aumanifest.h"

#include "effects/defs.h"

AUManifest::AUManifest(const QString& id, AVAudioUnitComponent* component) {
    setBackendType(EffectBackendType::AU);

    setId(id);
    setName(QString::fromNSString([component name]));
}
