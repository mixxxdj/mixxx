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
    virtual QWidget* buildWidget(QWidget* parent);

    virtual QJSValue value() const = 0;

    virtual QString stringify() const = 0;
    virtual void parse(const QString&, bool*) = 0;
    virtual bool isDefault() const = 0;
    virtual bool isDirty() const = 0;

    virtual void save() = 0;
    virtual void reset() = 0;

    // virtual void reset() const = 0;

    /// @brief Whether of not this setting definition is valid. Validity scope
    /// includes things like default value within range' for example.
    /// @return true if valid
    virtual inline bool valid() const {
        return !m_variableName.isEmpty();
    }

    /// @brief The variable name as perceived within the mapping definition.
    /// @return a string
    inline QString variableName() const {
        return m_variableName;
    }

    /// @brief The user-friendly label to be display in the UI
    /// @return a string
    inline const QString& label() const {
        return m_label;
    }

    /// @brief A description of what this setting does
    /// @return a string
    inline const QString& description() const {
        return m_description;
    }

  protected:
    AbstractLegacyControllerSetting(QString variableName, QString label, QString description)
            : m_variableName(variableName), m_label(label), m_description(description) {
    }
    AbstractLegacyControllerSetting(const QDomElement& element) {
        m_variableName = element.attribute("variable").trimmed();
        m_label = element.attribute("label", m_variableName).trimmed();

        QDomElement description = element.firstChildElement("description");
        if (description.isNull()) {
            m_description = QString();
            return;
        }
        m_description = description.text().trimmed();
    }

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

    QWidget* buildWidget(QWidget* parent) override;

    inline QJSValue value() const {
        return QJSValue(m_currentValue);
    }

    inline QString stringify() const {
        return m_currentValue ? "true" : "false";
    }
    inline void parse(const QString& in, bool* ok = nullptr) {
        if (ok != nullptr)
            *ok = true;
        m_currentValue = parseValue(in);
        m_dirtyValue = m_currentValue;
    }
    inline bool isDefault() const {
        return m_currentValue == m_defaultValue;
    }
    inline bool isDirty() const {
        return m_currentValue != m_dirtyValue;
    }

    virtual void save() {
        m_currentValue = m_dirtyValue;
    }

    virtual void reset() {
        m_currentValue = m_defaultValue;
        m_dirtyValue = m_defaultValue;
    }

    static inline AbstractLegacyControllerSetting* createFrom(const QDomElement& element) {
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

    inline bool parseValue(const QString& in) {
        return QString::compare(in, "true", Qt::CaseInsensitive) == 0 || in == "1";
    }

    virtual QWidget* buildInputWidget(QWidget* parent);

  private:
    bool m_currentValue;
    bool m_defaultValue;
    bool m_dirtyValue;
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

    inline QJSValue value() const {
        return QJSValue(m_currentValue);
    }

    inline QString stringify() const {
        return ValueSerializer(m_currentValue);
    }
    inline void parse(const QString& in, bool* ok) {
        m_currentValue = ValueDeserializer(in, ok);
        m_dirtyValue = m_currentValue;
    }

    inline bool isDefault() const {
        return m_currentValue == m_defaultValue;
    }
    inline bool isDirty() const {
        return m_currentValue != m_dirtyValue;
    }

    virtual void save() {
        m_currentValue = m_dirtyValue;
    }

    virtual void reset() {
        m_currentValue = m_defaultValue;
        m_dirtyValue = m_defaultValue;
    }

    static inline AbstractLegacyControllerSetting* createFrom(const QDomElement& element) {
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

    virtual QWidget* buildInputWidget(QWidget* parent);

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

class LegacyControllerRealSetting : public LegacyControllerNumberSetting<double,
                                            packSettingDoubleValue,
                                            extractSettingDoubleValue,
                                            QDoubleSpinBox> {
  public:
    LegacyControllerRealSetting(const QDomElement& element);

    static inline AbstractLegacyControllerSetting* createFrom(const QDomElement& element) {
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

    inline QJSValue value() const {
        return QJSValue(stringify());
    }

    QString stringify() const {
        return std::get<0>(m_options.value(m_currentValue));
    }
    void parse(const QString& in, bool* ok);
    bool isDefault() const {
        return m_currentValue == m_defaultValue;
    }
    inline bool isDirty() const {
        return m_currentValue != m_dirtyValue;
    }

    virtual void save() {
        m_currentValue = m_dirtyValue;
    }

    virtual void reset() {
        m_currentValue = m_defaultValue;
        m_dirtyValue = m_defaultValue;
    }

    static inline AbstractLegacyControllerSetting* createFrom(const QDomElement& element) {
        return new LegacyControllerEnumSetting(element);
    }
    static bool match(const QDomElement& element);

  protected:
    LegacyControllerEnumSetting(const QDomElement& element,
            QList<std::tuple<QString, QString>> options,
            size_t currentValue,
            size_t defaultValue)
            : AbstractLegacyControllerSetting(element),
              m_options(options),
              m_currentValue(currentValue),
              m_defaultValue(defaultValue) {
    }

    virtual QWidget* buildInputWidget(QWidget* parent);

  private:
    // We use a QList instead of QHash here because we want to keep the natural order
    QList<std::tuple<QString, QString>> m_options;
    size_t m_currentValue;
    size_t m_defaultValue;

    size_t m_dirtyValue;
};
