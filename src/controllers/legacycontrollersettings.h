#pragma once

#include <gtest/gtest_prod.h>
#include <qcolor.h>
#include <qglobal.h>
#include <qstringliteral.h>

#include <QColor>
#include <QFileInfo>
#include <QJSValue>
#include <QUrl>
#include <algorithm>

#include "controllers/legacycontrollersettingsfactory.h"
#include "controllers/legacycontrollersettingslayout.h"
#include "util/assert.h"
#ifdef MIXXX_USE_QML
#include <QQmlEngine>
#endif

namespace {
template<class T>
bool valid_range(T min, T max, T step) {
    VERIFY_OR_DEBUG_ASSERT(step >= 0) {
        return false;
    }
    return step > 0 && ((min <= 0 && min + step <= max) || (max >= 0 && max - step >= min));
}
} // namespace

class QSpinBox;
class QDoubleSpinBox;

/// @brief The abstract controller setting. Any type of setting will have to
/// implement this base class
class AbstractLegacyControllerSetting : public QObject {
    Q_OBJECT
#ifdef MIXXX_USE_QML
    Q_PROPERTY(QJSValue value READ value WRITE setValue NOTIFY changed)
    Q_PROPERTY(QString variableName READ variableName CONSTANT)
    Q_PROPERTY(QString label READ label CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QString type READ getType CONSTANT)
    QML_ANONYMOUS
#endif
  public:
    ~AbstractLegacyControllerSetting() override = default;

    /// @brief Build a widget that can be used to interact with this setting. It
    /// shouldn't mutate the state of the setting.
    /// @param parent The parent widget for which this widget is being created.
    /// The parent widget will own the newly created widget
    /// @return a new widget
    virtual QWidget* buildWidget(QWidget* parent,
            LegacyControllerSettingsLayoutContainer::Disposition orientation =
                    LegacyControllerSettingsLayoutContainer::Disposition::HORIZONTAL);

    /// @brief Build a JSValue with the current setting value. The JSValue
    /// variant will use the appropriate type
    /// @return A QJSValue with the current value
    Q_INVOKABLE virtual QJSValue value() const = 0;

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
    Q_INVOKABLE virtual bool isDefault() const = 0;

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
    Q_INVOKABLE const QString& label() const {
        return m_label;
    }

    /// @brief A description of what this setting does
    /// @return a string
    Q_INVOKABLE const QString& description() const {
        return m_description;
    }

    bool operator==(const AbstractLegacyControllerSetting& other) const noexcept {
        return variableName() == other.variableName();
    }

#ifdef MIXXX_USE_QML
    /// @brief Serialise a JSValue as the new setting value. The JSValue
    /// variant will use the appropriate type
    /// @param value A QJSValue with the new value
    Q_INVOKABLE virtual void setValue(const QJSValue& value) = 0;

    /// @brief Serialize the setting parameters
    /// @param doc The DOM document in which the parameter will be serialise
    /// @param e The DOM element in which to serialise parameter
    virtual void serialize(QDomDocument* doc, QDomElement* e) const;

    /// @brief The setting type in human readable form.
    /// @return a string
    virtual QString getType() const = 0;
#endif

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
    /// This signal will be emitted when the user has requested a value reset
    void valueReset();

  private:
    QString m_variableName;
    QString m_label;
    QString m_description;
};

/// @brief Base setting class generic
/// @tparam T The type of value this setting type is holding
template<class T>
class LegacyControllerSettingMixin : public AbstractLegacyControllerSetting {
  public:
    bool isDefault() const override {
        return m_savedValue == m_defaultValue;
    }
    bool isDirty() const override {
        return m_savedValue != m_editedValue;
    }

    void save() override {
        m_savedValue = m_editedValue;
    }

    void reset() override {
        m_editedValue = m_defaultValue;
        emit valueReset();
    }

  protected:
    LegacyControllerSettingMixin(const QDomElement& element)
            : AbstractLegacyControllerSetting(element) {
    }
    LegacyControllerSettingMixin(const QDomElement& element,
            T defaultValue)
            : AbstractLegacyControllerSetting(element),
              m_savedValue(defaultValue),
              m_defaultValue(defaultValue),
              m_editedValue(defaultValue) {
        reset();
        save();
    }
    LegacyControllerSettingMixin(const QDomElement& element,
            T currentValue,
            T defaultValue)
            : AbstractLegacyControllerSetting(element),
              m_savedValue(currentValue),
              m_defaultValue(defaultValue) {
    }

    T m_savedValue;
    T m_defaultValue;
    T m_editedValue;
};

class LegacyControllerBooleanSetting
        : public LegacyControllerSettingMixin<bool>,
          public LegacyControllerSettingFactory<LegacyControllerBooleanSetting> {
  public:
    LegacyControllerBooleanSetting(const QDomElement& element);

    virtual ~LegacyControllerBooleanSetting() = default;

    QWidget* buildWidget(QWidget* parent,
            LegacyControllerSettingsLayoutContainer::Disposition orientation =
                    LegacyControllerSettingsLayoutContainer::Disposition::HORIZONTAL)
            override;

    QJSValue value() const override {
        return QJSValue(m_editedValue);
    }

#ifdef MIXXX_USE_QML
    void setValue(const QJSValue& value) override {
        bool newValue = value.toBool();

        if (newValue == m_editedValue) {
            return;
        }
        m_editedValue = newValue;
        emit changed();
    }

    void serialize(QDomDocument* doc, QDomElement* e) const override;

    QString getType() const override {
        return QStringLiteral("boolean");
    }
#endif

    QString stringify() const override {
        return m_savedValue ? "true" : "false";
    }
    void parse(const QString& in, bool* ok = nullptr) override {
        if (ok != nullptr) {
            *ok = true;
        }
        m_savedValue = parseValue(in);
        m_editedValue = m_savedValue;
    }

    static std::shared_ptr<LegacyControllerBooleanSetting> createFrom(const QDomElement& element) {
        return std::make_shared<LegacyControllerBooleanSetting>(element);
    }
    static bool match(const QDomElement& element);

  protected:
    bool parseValue(const QString& in) {
        return QString::compare(in, "true", Qt::CaseInsensitive) == 0 || in == "1";
    }

    QWidget* buildInputWidget(QWidget* parent) override;

    FRIEND_TEST(LegacyControllerMappingSettingsTest, booleanSettingEditing);
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
          public LegacyControllerSettingMixin<SettingType> {
  public:
    using LegacyControllerSettingMixin<SettingType>::m_savedValue;
    using LegacyControllerSettingMixin<SettingType>::m_defaultValue;
    using LegacyControllerSettingMixin<SettingType>::m_editedValue;
    using LegacyControllerSettingMixin<SettingType>::save;
    using LegacyControllerSettingMixin<SettingType>::reset;
    using LegacyControllerSettingMixin<SettingType>::connect;
    using LegacyControllerSettingMixin<SettingType>::changed;
    LegacyControllerNumberSetting(const QDomElement& element)
            : LegacyControllerSettingMixin<SettingType>(element) {
        bool isOk = false;
        m_minValue = ValueDeserializer(element.attribute("min"), &isOk);
        if (!isOk) {
            m_minValue = std::numeric_limits<int>::min();
        }
        m_maxValue = ValueDeserializer(element.attribute("max"), &isOk);
        if (!isOk) {
            m_maxValue = std::numeric_limits<int>::max();
        }
        m_stepValue = ValueDeserializer(element.attribute("step"), &isOk);
        if (!isOk) {
            m_stepValue = 1;
        }
        m_defaultValue = ValueDeserializer(element.attribute("default"), &isOk);
        if (!isOk) {
            if (0 > m_maxValue) {
                m_defaultValue = m_maxValue;
            } else if (0 < m_minValue) {
                m_defaultValue = m_minValue;
            } else {
                m_defaultValue = 0;
            }
        }
        reset();
        save();
    }

    virtual ~LegacyControllerNumberSetting() = default;

    QJSValue value() const override {
        return QJSValue(m_editedValue);
    }

#ifdef MIXXX_USE_QML
    void setValue(const QJSValue& value) override {
        SettingType newValue = static_cast<SettingType>(value.toNumber());

        if (newValue == m_editedValue) {
            return;
        }
        m_editedValue = newValue;
        emit changed();
    }

    void serialize(QDomDocument* doc, QDomElement* e) const override;

    QString getType() const override {
        return QStringLiteral("number");
    }
#endif

    QString stringify() const override {
        return ValueSerializer(m_savedValue);
    }
    void parse(const QString& in, bool* ok) override {
        m_savedValue = ValueDeserializer(in, ok);
        m_editedValue = m_savedValue;
    }

    /// @brief Whether of not this setting definition and its current state are
    /// valid. Validity scope includes default/current/dirty value within range
    /// and a strictly positive step, strictly less than max..
    /// @return true if valid
    bool valid() const override {
        return AbstractLegacyControllerSetting::valid() &&
                m_defaultValue >= m_minValue && m_savedValue >= m_minValue &&
                m_editedValue >= m_minValue && m_defaultValue <= m_maxValue &&
                m_savedValue <= m_maxValue && m_editedValue <= m_maxValue &&
                valid_range(m_minValue, m_maxValue, m_stepValue);
    }

    static std::shared_ptr<LegacyControllerNumberSetting> createFrom(const QDomElement& element) {
        return std::make_shared<LegacyControllerNumberSetting>(element);
    }
    static bool match(const QDomElement& element);

  protected:
    LegacyControllerNumberSetting(const QDomElement& element,
            SettingType currentValue,
            SettingType defaultValue,
            SettingType minValue,
            SettingType maxValue,
            SettingType stepValue)
            : LegacyControllerSettingMixin<SettingType>(element,
                      currentValue,
                      defaultValue),
              m_minValue(minValue),
              m_maxValue(maxValue),
              m_stepValue(stepValue) {
    }

    QWidget* buildInputWidget(QWidget* parent) override;

  protected:
    SettingType m_minValue;
    SettingType m_maxValue;
    SettingType m_stepValue;

    FRIEND_TEST(LegacyControllerMappingSettingsTest, integerSettingEditing);
    FRIEND_TEST(LegacyControllerMappingSettingsTest, doubleSettingEditing);
};

template<class T>
inline bool matchSetting(const QDomElement& element);

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

class LegacyControllerIntegerSetting : public LegacyControllerNumberSetting<int,
                                               packSettingIntegerValue,
                                               extractSettingIntegerValue,
                                               QSpinBox> {
    Q_OBJECT
#ifdef MIXXX_USE_QML
    Q_PROPERTY(int min MEMBER m_minValue CONSTANT)
    Q_PROPERTY(int max MEMBER m_maxValue CONSTANT)
    Q_PROPERTY(int step MEMBER m_stepValue CONSTANT)
    QML_ANONYMOUS
#endif
  public:
    explicit LegacyControllerIntegerSetting(const QDomElement& element)
            : LegacyControllerNumberSetting(element) {
    }
    static std::shared_ptr<LegacyControllerIntegerSetting> createFrom(const QDomElement& element) {
        return std::make_shared<LegacyControllerIntegerSetting>(element);
    }
};

class LegacyControllerRealSetting : public LegacyControllerNumberSetting<double,
                                            packSettingDoubleValue,
                                            extractSettingDoubleValue,
                                            QDoubleSpinBox> {
    Q_OBJECT
#ifdef MIXXX_USE_QML
    Q_PROPERTY(double min MEMBER m_minValue CONSTANT)
    Q_PROPERTY(double max MEMBER m_maxValue CONSTANT)
    Q_PROPERTY(double step MEMBER m_stepValue CONSTANT)
    Q_PROPERTY(double precision MEMBER m_precisionValue CONSTANT)
    QML_ANONYMOUS
#endif
  public:
    LegacyControllerRealSetting(const QDomElement& element)
            : LegacyControllerNumberSetting(element) {
        bool isOk = false;
        m_precisionValue = element.attribute("precision").toInt(&isOk);
        if (!isOk) {
            m_precisionValue = 2;
        }
    }

    static std::shared_ptr<LegacyControllerRealSetting> createFrom(const QDomElement& element) {
        return std::make_shared<LegacyControllerRealSetting>(element);
    }

#ifdef MIXXX_USE_QML
    void serialize(QDomDocument* doc, QDomElement* e) const override;
#endif

    QWidget* buildInputWidget(QWidget* parent) override;

  private:
    int m_precisionValue;
};

struct LegacyControllerEnumItem {
#ifdef MIXXX_USE_QML
    Q_GADGET
    Q_PROPERTY(QString value MEMBER value CONSTANT)
    Q_PROPERTY(QString label MEMBER label CONSTANT)
    Q_PROPERTY(QColor color MEMBER color CONSTANT)
#endif
  public:
    QString value;
    QString label;
    QColor color;
};
class LegacyControllerEnumSetting
        : public LegacyControllerSettingMixin<qsizetype>,
          public LegacyControllerSettingFactory<LegacyControllerEnumSetting> {
#ifdef MIXXX_USE_QML
    Q_OBJECT
    Q_PROPERTY(QList<LegacyControllerEnumItem> options MEMBER m_options CONSTANT)
    Q_PROPERTY(qsizetype currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY changed)
    QML_ANONYMOUS
#endif
  public:

    LegacyControllerEnumSetting(const QDomElement& element);

    virtual ~LegacyControllerEnumSetting() = default;

    QJSValue value() const override {
        return QJSValue(stringify());
    }

#ifdef MIXXX_USE_QML
    void setValue(const QJSValue& rawValue) override {
        auto value = rawValue.toString();
        auto newValue = std::distance(m_options.cbegin(),
                std::find_if(m_options.cbegin(),
                        m_options.cend(),
                        [value](const LegacyControllerEnumItem& item) {
                            return item.value == value;
                        }));
        setCurrentIndex(newValue);
    }

    void serialize(QDomDocument* doc, QDomElement* e) const override;

    QString getType() const override {
        return QStringLiteral("enum");
    }
#endif

    Q_INVOKABLE void setCurrentIndex(qsizetype index) {
        VERIFY_OR_DEBUG_ASSERT(index >= 0 && index < m_options.size()) {
            return;
        }
        if (m_editedValue == index) {
            return;
        }
        m_editedValue = index;
        emit changed();
    }

    qsizetype currentIndex() const {
        return m_editedValue;
    }

    const QList<LegacyControllerEnumItem>& options() const {
        return m_options;
    }

    QString stringify() const override {
        return m_options.value(static_cast<int>(m_editedValue)).value;
    }
    void parse(const QString& in, bool* ok) override;

    /// @brief Whether or not this setting definition and its current state are
    /// valid. Validity scope includes a known default/current/dirty option.
    /// @return true if valid
    bool valid() const override {
        return AbstractLegacyControllerSetting::valid() &&
                static_cast<int>(m_defaultValue) < m_options.size() &&
                static_cast<int>(m_savedValue) < m_options.size() &&
                static_cast<int>(m_editedValue) < m_options.size();
    }

    static std::shared_ptr<LegacyControllerEnumSetting> createFrom(const QDomElement& element) {
        return std::make_shared<LegacyControllerEnumSetting>(element);
    }
    static inline bool match(const QDomElement& element);

  protected:
    LegacyControllerEnumSetting(const QDomElement& element,
            const QList<LegacyControllerEnumItem>& options,
            size_t currentValue,
            size_t defaultValue)
            : LegacyControllerSettingMixin(element, currentValue, defaultValue),
              m_options(options) {
    }

    QWidget* buildInputWidget(QWidget* parent) override;

  private:
    // We use a QList instead of QHash here because we want to keep the natural order
    QList<LegacyControllerEnumItem> m_options;

    FRIEND_TEST(LegacyControllerMappingSettingsTest, enumSettingEditing);
    FRIEND_TEST(ControllerS4MK3SettingTest, ensureLibrarySettingValueAndEnumEquals);
};

class LegacyControllerColorSetting
        : public LegacyControllerSettingMixin<QColor>,
          public LegacyControllerSettingFactory<LegacyControllerColorSetting> {
  public:
    LegacyControllerColorSetting(const QDomElement& element);

    ~LegacyControllerColorSetting() override;

    QJSValue value() const override {
        return QJSValue(stringify());
    }

#ifdef MIXXX_USE_QML
    void setValue(const QJSValue& rawValue) override {
        QColor value = rawValue.toVariant().value<QColor>();
        if (value == m_editedValue) {
            return;
        }
        m_editedValue = value;
        emit changed();
    }

    void serialize(QDomDocument* doc, QDomElement* e) const override;

    QString getType() const override {
        return QStringLiteral("color");
    }
#endif

    QString stringify() const override {
        return m_editedValue.name(QColor::HexRgb);
    }
    void parse(const QString& in, bool* ok) override;

    /// @brief Whether or not this setting definition and its current state are
    /// valid. Validity scope includes a known default/current/dirty option.
    /// @return true if valid
    bool valid() const override {
        return AbstractLegacyControllerSetting::valid() &&
                m_defaultValue.isValid() &&
                m_savedValue.isValid();
    }

    static std::shared_ptr<LegacyControllerColorSetting> createFrom(const QDomElement& element) {
        return std::make_shared<LegacyControllerColorSetting>(element);
    }
    static inline bool match(const QDomElement& element) {
        return element.hasAttribute(QStringLiteral("type")) &&
                QString::compare(element.attribute(QStringLiteral("type")),
                        QStringLiteral("color"),
                        Qt::CaseInsensitive) == 0;
    }

  protected:
    LegacyControllerColorSetting(const QDomElement& element,
            QColor currentValue,
            QColor defaultValue)
            : LegacyControllerSettingMixin(element,
                      defaultValue,
                      currentValue) {
    }

    QWidget* buildInputWidget(QWidget* parent) override;

    FRIEND_TEST(ControllerS4MK3SettingTest, ensureLibrarySettingValueAndEnumEquals);
    FRIEND_TEST(LegacyControllerMappingSettingsTest, enumSettingEditing);
};

class LegacyControllerFileSetting
        : public LegacyControllerSettingMixin<QFileInfo>,
          public LegacyControllerSettingFactory<LegacyControllerFileSetting> {
#ifdef MIXXX_USE_QML
    Q_OBJECT
    Q_PROPERTY(QString fileFilter MEMBER m_fileFilter CONSTANT)
    QML_ANONYMOUS
#endif
  public:
    LegacyControllerFileSetting(const QDomElement& element);

    virtual ~LegacyControllerFileSetting();

    QJSValue value() const override {
        return QJSValue(stringify());
    }

#ifdef MIXXX_USE_QML
    void setValue(const QJSValue& rawValue) override {
        QUrl path = rawValue.toVariant().value<QUrl>();
        QFileInfo value;
        if (!path.isValid()) {
            value = QFileInfo();
        } else {
            value = QFileInfo(path.toLocalFile());
            DEBUG_ASSERT(value.exists());
        }
        if (value == m_editedValue) {
            return;
        }
        m_editedValue = value;
        emit changed();
    }

    void serialize(QDomDocument* doc, QDomElement* e) const override;

    QString getType() const override {
        return QStringLiteral("file");
    }

#endif

    QString stringify() const override {
        return m_editedValue.absoluteFilePath();
    }
    void parse(const QString& in, bool* ok) override;

    /// @brief Whether or not this setting definition and its current state are
    /// valid. Validity scope includes a known default/current/dirty option.
    /// @return true if valid
    bool valid() const override {
        return AbstractLegacyControllerSetting::valid() &&
                (m_defaultValue == m_savedValue || m_savedValue.exists());
    }

    static std::shared_ptr<LegacyControllerFileSetting> createFrom(const QDomElement& element) {
        return std::make_shared<LegacyControllerFileSetting>(element);
    }
    static bool match(const QDomElement& element) {
        return element.hasAttribute(QStringLiteral("type")) &&
                QString::compare(element.attribute(QStringLiteral("type")),
                        QStringLiteral("file"),
                        Qt::CaseInsensitive) == 0;
    }

  protected:
    LegacyControllerFileSetting(const QDomElement& element,
            const QFileInfo& currentValue,
            const QFileInfo& defaultValue)
            : LegacyControllerSettingMixin(element,
                      defaultValue,
                      currentValue) {
    }

    QWidget* buildInputWidget(QWidget* parent) override;

  private:
    QString m_fileFilter;

    FRIEND_TEST(LegacyControllerMappingSettingsTest, enumSettingEditing);
    FRIEND_TEST(ControllerS4MK3SettingTest, ensureLibrarySettingValueAndEnumEquals);
};

template<>
inline bool matchSetting<int>(const QDomElement& element) {
    return element.hasAttribute("type") &&
            QString::compare(element.attribute("type"),
                    "integer",
                    Qt::CaseInsensitive) == 0;
}
template<>
inline bool matchSetting<double>(const QDomElement& element) {
    return element.hasAttribute("type") &&
            QString::compare(element.attribute("type"),
                    "real",
                    Qt::CaseInsensitive) == 0;
}

template<class SettingType,
        Serializer<SettingType> ValueSerializer,
        Deserializer<SettingType> ValueDeserializer,
        class InputWidget>
inline bool LegacyControllerNumberSetting<SettingType,
        ValueSerializer,
        ValueDeserializer,
        InputWidget>::match(const QDomElement& element) {
    return matchSetting<SettingType>(element);
}

inline bool LegacyControllerEnumSetting::match(const QDomElement& element) {
    return element.hasAttribute("type") &&
            QString::compare(element.attribute("type"),
                    "enum",
                    Qt::CaseInsensitive) == 0;
}
