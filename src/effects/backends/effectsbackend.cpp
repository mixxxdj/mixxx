#include "effects/backends/effectsbackend.h"

namespace {
const QString backendTypeNameLV2 = QStringLiteral("LV2");
// QString::tr requires const char[] rather than QString
constexpr char backendTypeNameBuiltIn[] = "Built-In";
constexpr char backendTypeNameUnknown[] = "Unknown";
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

QString EffectsBackend::translatedBackendName(EffectBackendType backendType) {
    switch (backendType) {
    case EffectBackendType::BuiltIn:
        //: Used for effects that are built into Mixxx
        return QObject::tr(backendTypeNameBuiltIn);
    case EffectBackendType::LV2:
        return backendTypeNameLV2;
    default:
        return QObject::tr(backendTypeNameUnknown);
    }
}
