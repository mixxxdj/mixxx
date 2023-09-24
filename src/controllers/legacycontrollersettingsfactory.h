#pragma once

#include <QDomElement>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QObject>
#include <QSizePolicy>
#include <QSpinBox>
#include <QString>
#include <QWidget>
#include <limits>
#include <memory>
#include <tuple>

#include "controllers/legacycontrollersettingsfactory.h"

class AbstractLegacyControllerSetting;

/// @brief This class defines an interface that a controller setting type must
/// implement so it can be used properly by the builder
/// @tparam T The class implementing this interface
template<class T>
class LegacyControllerSettingFactory {
    inline static LegacyControllerSettingFactory* createFrom(const QDomElement& element) {
        return new T(element);
    }
    inline static bool match(const QDomElement& element) {
        return T::match(element);
    }
};

/// @brief This class is used to dynamically instantiate a controller setting based on its type
class LegacyControllerSettingBuilder {
  public:
    static LegacyControllerSettingBuilder* instance();

    /// @brief Register a new type of setting. This method is used by the
    /// REGISTER macro, it shouldn't be used directly
    /// @param match the match function of the new setting
    /// @param creator the creator function of the new setting
    /// @return Always true
    bool registerType(bool (*match)(const QDomElement&),
            AbstractLegacyControllerSetting* (*creator)(const QDomElement&)) {
        m_supportedSettings.append(std::make_tuple(match, creator));
        return true;
    }

    /// @brief instantiate a new setting from a an XML definition if any valid
    /// setting was found. The caller is the owner of the instance
    /// @param element The XML element to parse to build the new setting
    /// @return an instance if a a supported setting has been found, null
    /// otherwise
    static AbstractLegacyControllerSetting* build(const QDomElement& element) {
        for (const auto& settingType : qAsConst(instance()->m_supportedSettings)) {
            if (std::get<0>(settingType)(element)) {
                return std::get<1>(settingType)(element);
            }
        }

        return nullptr;
    }

  private:
    LegacyControllerSettingBuilder() = default;

    QList<std::tuple<bool (*)(const QDomElement&),
            AbstractLegacyControllerSetting* (*)(const QDomElement&)>>
            m_supportedSettings;

    static LegacyControllerSettingBuilder* __self;
};

#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)
#define REGISTER_LEGACY_CONTROLLER_SETTING(...)                       \
    bool CONCAT(kSettingRegistered_, __COUNTER__) =                   \
            LegacyControllerSettingBuilder::instance()->registerType( \
                    __VA_ARGS__::match, __VA_ARGS__::createFrom)
