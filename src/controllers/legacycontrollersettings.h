#pragma once

#include <QDomElement>
#include <QJSValue>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QLocale>
#include <QObject>
#include <QSizePolicy>
#include <QSpinBox>
#include <QString>
#include <QWidget>
#include <limits>
#include <memory>
#include <tuple>

#include "controllers/legacycontrollersettingsfactory.h"
#include "controllers/legacycontrollersettingslayout.h"

/// @brief The abstract controller setting. Any type of setting will have to
/// implement this base class
class AbstractLegacyControllerSetting : public QObject {
    Q_OBJECT
  public:
    virtual ~AbstractLegacyControllerSetting() = default;

    /// @brief Build a widget that can be used to interact with this setting. It
    /// shouldn't mutate the state of the setting.
    /// @param parent The parent widget for which this widget is being created.
    /// The parent widget will own the newly created widget
    /// @return a new widget
    virtual QWidget* buildWidget(QWidget* parent,
            LegacyControllerSettingsLayoutContainer::Disposition orientation =
                    LegacyControllerSettingsLayoutContainer::HORIZONTAL);

    /// @brief Build a JSValue with the current setting value. The JSValue
    /// variant will use the appropriate type
    /// @return A QJSValue with the current value
    virtual QJSValue value() const = 0;

    /// @brief Serialize the current value in a string format
    /// @return A String with current setting value
    virtual QString stringify() const = 0;

    /// @brief Parse a string that contains the value in a compatible format.
    /// @param in The string containing the data
    /// @param ok A pointer to a boolean in which the result of the
    /// deserialisation will be stored (true means the operation was successful)
    virtual void parse(const QString&, bool*) = 0;

    /// @brief Indicate if the setting is currently not using a user-specified value
    /// @return Whether or not the setting is currently set to its default value
    virtual bool isDefault() const = 0;

    /// @brief Indicate if the setting is currently being mutated and if the
    /// edited value is different than its its currently known value. This would
    /// indicate that the user may need to save the changes or acknowledge
    /// otherwise.
    /// @return Whether or not the setting value is dirty
    virtual bool isDirty() const = 0;

    /// @brief Commit the the edited value to become the currently known value.
    /// Note that if `isDirty() == false`, this have no effect
    virtual void save() = 0;

    /// @brief Reset the current value, as well as the editing value to use the
    /// default, as specified in the spec
    virtual void reset() = 0;

    /// @brief Whether of not this setting definition is valid. Validity scope
    /// includes things like default value within range' for example.
    /// @return true if valid
    virtual bool valid() const {
        return !m_variableName.isEmpty();
    }

    /// @brief The variable name as perceived within the mapping definition.
    /// @return a string
    QString variableName() const {
        return m_variableName;
    }

    /// @brief The user-friendly label to be display in the UI
    /// @return a string
    const QString& label() const {
        return m_label;
    }

    /// @brief A description of what this setting does
    /// @return a string
    const QString& description() const {
        return m_description;
    }

  protected:
    AbstractLegacyControllerSetting(const QString& variableName,
            const QString& label,
            const QString& description)
            : m_variableName(variableName),
              m_label(label),
              m_description(description) {
    }
    AbstractLegacyControllerSetting(const QDomElement& element);

    virtual QWidget* buildInputWidget(QWidget* parent) = 0;

  signals:
    /// This signal will be emitted when the user has interacted with the
    /// setting and changed its value
    void changed();

  private:
    QString m_variableName;
    QString m_label;
    QString m_description;
};

class LegacyControllerBooleanSetting
        : public LegacyControllerSettingFactory<LegacyControllerBooleanSetting>,
          public AbstractLegacyControllerSetting {
  public:
    LegacyControllerBooleanSetting(const QDomElement& element);

    virtual ~LegacyControllerBooleanSetting() = default;

    QWidget* buildWidget(QWidget* parent,
            LegacyControllerSettingsLayoutContainer::Disposition orientation =
                    LegacyControllerSettingsLayoutContainer::HORIZONTAL)
            override;

    QJSValue value() const override {
        return QJSValue(m_currentValue);
    }

    QString stringify() const override {
        return m_currentValue ? "true" : "false";
    }
    void parse(const QString& in, bool* ok = nullptr) override {
        if (ok != nullptr)
            *ok = true;
        m_currentValue = parseValue(in);
        m_dirtyValue = m_currentValue;
    }
    bool isDefault() const override {
        return m_currentValue == m_defaultValue;
    }
    bool isDirty() const override {
        return m_currentValue != m_dirtyValue;
    }

    virtual void save() override {
        m_currentValue = m_dirtyValue;
    }

    virtual void reset() override {
        m_currentValue = m_defaultValue;
        m_dirtyValue = m_defaultValue;
    }

    static AbstractLegacyControllerSetting* createFrom(const QDomElement& element) {
        return new LegacyControllerBooleanSetting(element);
    }
    static bool match(const QDomElement& element);

  protected:
    LegacyControllerBooleanSetting(const QDomElement& element,
            bool currentValue,
            bool defaultValue)
            : AbstractLegacyControllerSetting(element),
              m_currentValue(currentValue),
              m_defaultValue(defaultValue) {
    }

    bool parseValue(const QString& in) {
        return QString::compare(in, "true", Qt::CaseInsensitive) == 0 || in == "1";
    }

    virtual QWidget* buildInputWidget(QWidget* parent) override;

  private:
    bool m_currentValue;
    bool m_defaultValue;
    bool m_dirtyValue;

    friend class LegacyControllerMappingSettingsTest_booleanSettingEditing_Test;
};

template<class SettingType>
using Serializer = QString (*)(const SettingType&);

template<class SettingType>
using Deserializer = SettingType (*)(const QString&, bool*);

template<class SettingType,
        Serializer<SettingType> ValueSerializer,
        Deserializer<SettingType> ValueDeserializer,
        class InputWidget>
class LegacyControllerNumberSetting
        : public LegacyControllerSettingFactory<
                  LegacyControllerNumberSetting<SettingType,
                          ValueSerializer,
                          ValueDeserializer,
                          InputWidget>>,
          public AbstractLegacyControllerSetting {
  public:
    LegacyControllerNumberSetting(const QDomElement& element);

    virtual ~LegacyControllerNumberSetting() = default;

    QJSValue value() const override {
        return QJSValue(m_currentValue);
    }

    QString stringify() const override {
        return ValueSerializer(m_currentValue);
    }
    void parse(const QString& in, bool* ok) override {
        m_currentValue = ValueDeserializer(in, ok);
        m_dirtyValue = m_currentValue;
    }

    bool isDefault() const override {
        return m_currentValue == m_defaultValue;
    }
    bool isDirty() const override {
        return m_currentValue != m_dirtyValue;
    }

    virtual void save() override {
        m_currentValue = m_dirtyValue;
    }

    virtual void reset() override {
        m_currentValue = m_defaultValue;
        m_dirtyValue = m_defaultValue;
    }

    /// @brief Whether of not this setting definition and its current state are
    /// valid. Validity scope includes default/current/dirty value within range
    /// and a strictly positive step, strictly less than max..
    /// @return true if valid
    bool valid() const override {
        return AbstractLegacyControllerSetting::valid() &&
                m_defaultValue >= m_minValue && m_currentValue >= m_minValue &&
                m_dirtyValue >= m_minValue && m_defaultValue <= m_maxValue &&
                m_currentValue <= m_maxValue && m_dirtyValue <= m_maxValue &&
                m_stepValue > 0 && m_stepValue < m_maxValue;
    }

    static AbstractLegacyControllerSetting* createFrom(const QDomElement& element) {
        return new LegacyControllerNumberSetting(element);
    }
    static bool match(const QDomElement& element);

  protected:
    LegacyControllerNumberSetting(const QDomElement& element,
            SettingType currentValue,
            SettingType defaultValue,
            SettingType minValue,
            SettingType maxValue,
            SettingType stepValue)
            : AbstractLegacyControllerSetting(element),
              m_currentValue(currentValue),
              m_defaultValue(defaultValue),
              m_minValue(minValue),
              m_maxValue(maxValue),
              m_stepValue(stepValue) {
    }

    virtual QWidget* buildInputWidget(QWidget* parent) override;

  private:
    SettingType m_currentValue;
    SettingType m_defaultValue;
    SettingType m_minValue;
    SettingType m_maxValue;
    SettingType m_stepValue;

    SettingType m_dirtyValue;
};

template<class T>
bool matchSetting(const QDomElement& element);

inline int extractSettingIntegerValue(const QString& str, bool* ok = nullptr) {
    return str.toInt(ok);
}
inline double extractSettingDoubleValue(const QString& str, bool* ok = nullptr) {
    return str.toDouble(ok);
}

inline QString packSettingIntegerValue(const int& in) {
    return QString::number(in);
}
inline QString packSettingDoubleValue(const double& in) {
    return QString::number(in);
}

using LegacyControllerIntegerSetting = LegacyControllerNumberSetting<int,
        packSettingIntegerValue,
        extractSettingIntegerValue,
        QSpinBox>;

class LegacyControllerRealSetting : public LegacyControllerNumberSetting<double,
                                            packSettingDoubleValue,
                                            extractSettingDoubleValue,
                                            QDoubleSpinBox> {
  public:
    LegacyControllerRealSetting(const QDomElement& element);

    static AbstractLegacyControllerSetting* createFrom(const QDomElement& element) {
        return new LegacyControllerRealSetting(element);
    }

    QWidget* buildInputWidget(QWidget* parent) override;

  private:
    int m_precisionValue;
};

class LegacyControllerEnumSetting
        : public LegacyControllerSettingFactory<LegacyControllerEnumSetting>,
          public AbstractLegacyControllerSetting {
  public:
    LegacyControllerEnumSetting(const QDomElement& element);

    virtual ~LegacyControllerEnumSetting() = default;

    QJSValue value() const override {
        return QJSValue(stringify());
    }

    QString stringify() const override {
        return std::get<0>(m_options.value(m_currentValue));
    }
    void parse(const QString& in, bool* ok) override;
    bool isDefault() const override {
        return m_currentValue == m_defaultValue;
    }
    bool isDirty() const override {
        return m_currentValue != m_dirtyValue;
    }

    virtual void save() override {
        m_currentValue = m_dirtyValue;
    }

    virtual void reset() override {
        m_currentValue = m_defaultValue;
        m_dirtyValue = m_defaultValue;
    }

    /// @brief Whether of not this setting definition and its current state are
    /// valid. Validity scope includes a known default/current/dirty option.
    /// @return true if valid
    bool valid() const override {
        return AbstractLegacyControllerSetting::valid() &&
                (int)m_defaultValue < m_options.size() &&
                (int)m_currentValue < m_options.size() &&
                (int)m_dirtyValue < m_options.size();
    }

    static AbstractLegacyControllerSetting* createFrom(const QDomElement& element) {
        return new LegacyControllerEnumSetting(element);
    }
    static bool match(const QDomElement& element);

  protected:
    LegacyControllerEnumSetting(const QDomElement& element,
            const QList<std::tuple<QString, QString>>& options,
            size_t currentValue,
            size_t defaultValue)
            : AbstractLegacyControllerSetting(element),
              m_options(options),
              m_currentValue(currentValue),
              m_defaultValue(defaultValue) {
    }

    virtual QWidget* buildInputWidget(QWidget* parent) override;

  private:
    // We use a QList instead of QHash here because we want to keep the natural order
    QList<std::tuple<QString, QString>> m_options;
    size_t m_currentValue;
    size_t m_defaultValue;

    size_t m_dirtyValue;
};
