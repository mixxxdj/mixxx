#include "effects/effectchainmixmode.h"

namespace {
const QString kDrySlashWetString = QStringLiteral("DRY/WET");
const QString kDryPlusWetString = QStringLiteral("DRY+WET");
} // anonymous namespace

QString EffectChainMixMode::toString(EffectChainMixMode::Type type) {
    if (type == EffectChainMixMode::DryPlusWet) {
        return kDryPlusWetString;
    }
    return kDrySlashWetString;
}

EffectChainMixMode::Type EffectChainMixMode::fromString(const QString& string) {
    if (string == kDryPlusWetString) {
        return Type::DryPlusWet;
    }
    return Type::DrySlashWet;
}
