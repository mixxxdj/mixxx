#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QQmlParserStatus>
#include <memory>

#include "control/controlbuttonmode.h"
#include "control/controlpushbutton.h"

namespace mixxx {
namespace qml {

class QmlSkinControlCreator : public QObject, public QQmlParserStatus {
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString group READ getGroup WRITE setGroup NOTIFY groupChanged REQUIRED)
    Q_PROPERTY(QString key READ getKey WRITE setKey NOTIFY keyChanged REQUIRED)
    Q_PROPERTY(bool persist READ getPersist WRITE setPersist NOTIFY persistChanged)
    Q_PROPERTY(double defaultValue READ getDefaultValue WRITE setDefaultValue
                    NOTIFY defaultValueChanged)
    Q_PROPERTY(ButtonMode buttonMode READ getButtonMode WRITE setButtonMode
                    NOTIFY buttonModeChanged)
    QML_NAMED_ELEMENT(SkinControlCreator)

  public:
    enum class ButtonMode {
        Push,
        Toggle,
        PowerWindow,
        LongPressLatching,
        Trigger,
    };
    Q_ENUM(ButtonMode)

    explicit QmlSkinControlCreator(QObject* parent = nullptr);

    void classBegin() override {
    }
    void componentComplete() override;

    void setGroup(const QString& group);
    const QString& getGroup() const;

    void setKey(const QString& key);
    const QString& getKey() const;

    void setPersist(bool persist);
    bool getPersist() const;

    void setDefaultValue(double defaultValue);
    double getDefaultValue() const;

    void setButtonMode(ButtonMode buttonMode);
    ButtonMode getButtonMode() const;

  signals:
    void groupChanged(const QString& group);
    void keyChanged(const QString& key);
    void persistChanged(bool persist);
    void defaultValueChanged(double defaultValue);
    void buttonModeChanged(ButtonMode buttonMode);

  private:
    static mixxx::control::ButtonMode toControlButtonMode(ButtonMode buttonMode);
    void createControl(bool allowBeforeComponentComplete = false);
    void createDefaultControlBeforeComponentComplete();

    ConfigKey m_key;
    bool m_persist;
    bool m_persistConfigured;
    double m_defaultValue;
    ButtonMode m_buttonMode;
    bool m_isComponentComplete;
    std::unique_ptr<ControlPushButton> m_pControl;
};

} // namespace qml
} // namespace mixxx
