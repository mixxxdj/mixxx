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
    virtual QWidget* buildWidget(QWidget* parent) = 0;

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

  signals:
    /// This signal will be emitted when the user has interacted with the
    /// setting and changed its value
    void changed();

  private:
    QString m_variableName;
    QString m_label;
    QString m_description;
};

class LegacyControllerIntegerSetting
        : public LegacyControllerSettingFactory<LegacyControllerIntegerSetting>,
          public AbstractLegacyControllerSetting {
  public:
    LegacyControllerIntegerSetting(const QDomElement& element,
            int currentValue,
            int defaultValue,
            int minValue,
            int maxValue,
            int stepValue)
            : AbstractLegacyControllerSetting(element),
              m_currentValue(currentValue),
              m_defaultValue(defaultValue),
              m_minValue(minValue),
              m_maxValue(maxValue),
              m_stepValue(stepValue) {
    }

    virtual ~LegacyControllerIntegerSetting() = default;

    QWidget* buildWidget(QWidget* parent);

    static AbstractLegacyControllerSetting* createFrom(const QDomElement& element);
    static bool match(const QDomElement& element);

  private:
    int m_currentValue;
    int m_defaultValue;
    int m_minValue;
    int m_maxValue;
    int m_stepValue;
};
