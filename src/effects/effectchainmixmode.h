#pragma once
#include <QString>

class EffectChainMixMode {
  public:
    enum Type {
        DrySlashWet = 0,
        DryPlusWet = 1
    };
    static constexpr int kNumModes = 2;

    static QString toString(Type type);
    static Type fromString(const QString& string);
};
