#include "effects/backends/effectsbackend.h"

namespace {
const QString backendTypeNameLV2 = QStringLiteral("LV2");
const QString backendTypeNameBuiltIn = QStringLiteral("BuiltIn");
const QString backendTypeNameUnknown = QStringLiteral("Unknown");
} // anonymous namespace

EffectBackendType EffectsBackend::backendTypeFromString(const QString& typeName) {
    if (typeName == backendTypeNameLV2) {
        return EffectBackendType::LV2;
    } else {
        return EffectBackendType::BuiltIn;
    }
}

QString EffectsBackend::backendTypeToString(EffectBackendType backendType) {
    switch (backendType) {
    case EffectBackendType::BuiltIn:
        return backendTypeNameBuiltIn;
    case EffectBackendType::LV2:
        return backendTypeNameLV2;
    default:
        return backendTypeNameUnknown;
    }
}
