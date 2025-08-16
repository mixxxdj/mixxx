#pragma once

#include <QDomElement>

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
    template<class T>
    bool registerType() {
        m_supportedSettings.append(SupportedSetting{
                &T::match,
                &T::createFrom});
        return true;
    }

    /// @brief instantiate a new setting from a an XML definition if any valid
    /// setting was found. The caller is the owner of the instance
    /// @param element The XML element to parse to build the new setting
    /// @return an instance if a a supported setting has been found, null
    /// otherwise
    static std::shared_ptr<AbstractLegacyControllerSetting> build(const QDomElement& element) {
        for (const auto& settingType : std::as_const(instance()->m_supportedSettings)) {
            if (settingType.matcher(element)) {
                return settingType.builder(element);
            }
        }

        return nullptr;
    }

  private:
    struct SupportedSetting {
        std::function<bool(const QDomElement&)> matcher;
        std::function<std::shared_ptr<AbstractLegacyControllerSetting>(const QDomElement&)> builder;
    };

    LegacyControllerSettingBuilder();

    QList<SupportedSetting>
            m_supportedSettings;
};
