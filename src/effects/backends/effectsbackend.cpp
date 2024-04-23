#include "effects/backends/effectsbackend.h"

#include <QObject>

namespace {
const QString backendTypeNameAudioUnit = QStringLiteral("AudioUnit");
const QString backendTypeNameLV2 = QStringLiteral("LV2");
// QString::tr requires const char[] rather than QString
//: Backend type for effects that are built into Mixxx.
constexpr char backendTypeNameBuiltIn[] = QT_TRANSLATE_NOOP("EffectsBackend", "Built-In");
//: Backend type for effects were the backend is unknown.
constexpr char backendTypeNameUnknown[] = QT_TRANSLATE_NOOP("EffectsBackend", "Unknown");
} // anonymous namespace

EffectBackendType EffectsBackend::backendTypeFromString(const QString& typeName) {
    if (typeName == backendTypeNameLV2) {
        return EffectBackendType::LV2;
    } else if (typeName == backendTypeNameAudioUnit) {
        return EffectBackendType::AudioUnit;
    } else {
        return EffectBackendType::BuiltIn;
    }
}

QString EffectsBackend::backendTypeToString(EffectBackendType backendType) {
    switch (backendType) {
    case EffectBackendType::BuiltIn:
        return backendTypeNameBuiltIn;
    case EffectBackendType::AudioUnit:
        return backendTypeNameAudioUnit;
    case EffectBackendType::LV2:
        return backendTypeNameLV2;
    default:
        return backendTypeNameUnknown;
    }
}

QString EffectsBackend::translatedBackendName(EffectBackendType backendType) {
    switch (backendType) {
    case EffectBackendType::BuiltIn:
        // Clazy's `tr-non-literal` check is a false positive, because the
        // source string has been marked `QT_TR_NOOP`.
        return QObject::tr(backendTypeNameBuiltIn); // clazy:exclude=tr-non-literal
    case EffectBackendType::AudioUnit:
        return backendTypeNameAudioUnit;
    case EffectBackendType::LV2:
        return backendTypeNameLV2;
    default:
        // Clazy's `tr-non-literal` check is a false positive, because the
        // source string has been marked `QT_TR_NOOP`.
        return QObject::tr(backendTypeNameUnknown); // clazy:exclude=tr-non-literal
    }
}
