#pragma once

#include <QList>
#include <QString>
#include <QWidget>
#include <memory>
#include <vector>

#include "preferences/configobject.h"
#include "util/string.h"

class ControlWidgetPropertyConnection;
class ControlParameterWidgetConnection;

class WBaseWidget {
  public:
    explicit WBaseWidget(QWidget* pWidget);
    virtual ~WBaseWidget();

    enum class ConnectionSide {
        None,
        Left,
        Right
    };

    virtual void Init();

    QWidget* toQWidget() const {
        return m_pWidget;
    }

    void appendBaseTooltip(const QString& tooltip) {
        m_baseTooltip.append(mixxx::removeTrailingWhitespaces(tooltip));
        updateBaseTooltipOptShortcuts();
    }

    void prependBaseTooltip(const QString& tooltip) {
        m_baseTooltip.prepend(mixxx::removeTrailingWhitespaces(tooltip));
        updateBaseTooltipOptShortcuts();
    }

    void setBaseTooltip(const QString& tooltip) {
        m_baseTooltip = mixxx::removeTrailingWhitespaces(tooltip);
        updateBaseTooltipOptShortcuts();
    }

    void setShortcutTooltip(const QString& tooltip) {
        // While the other set functions are called only during widget construction,
        // this is also called each time the keyboard config file is reloaded,
        // so this may be called even though this widget's shortcuts are unchanged
        // and/or while we currently don't show shortcuts, so just update when
        // we need in order to prevent thousands of no-ops.
        bool reloadNow = m_showKeyboardShortcuts && m_shortcutTooltip != tooltip;
        m_shortcutTooltip = tooltip;
        if (reloadNow) {
            updateBaseTooltipOptShortcuts();
        }
    }

    QString baseTooltip() const {
        return m_baseTooltip;
    }

    void addConnection(
            std::unique_ptr<ControlParameterWidgetConnection> pConnection,
            ConnectionSide side);

    void addPropertyConnection(std::unique_ptr<ControlWidgetPropertyConnection> pConnection);

    // Add a ControlWidgetConnection to be the display connection for the
    // widget. There can only be one DisplayConnection. Calling this multiple
    // times will add each connection but only the last one is considered to be
    // the DisplayConnection.
    void addAndSetDisplayConnection(
            std::unique_ptr<ControlParameterWidgetConnection> pConnection,
            ConnectionSide side);

    double getControlParameter() const;
    double getControlParameterLeft() const;
    double getControlParameterRight() const;
    double getControlParameterDisplay() const;

    QString shortcutHints() const {
        return m_shortcutTooltip;
    }

    QString baseTooltipOptShortcuts() const {
        return m_baseTooltipOptShortcuts;
    }

    /// Set by LegacySkinParser on widget construction
    void setShortcutControlsAndCommands(
            const QList<std::pair<ConfigKey, QString>>& controlsCommands) {
        m_shortcutControlsAndCommands = controlsCommands;
    }

    /// Called by KeyboardEventFilter::updateWidgetShortcuts() when the keyboard
    /// config has been (re)loaded in order to update the shortcut tooltips
    const QList<std::pair<ConfigKey, QString>>& getShortcutControlsAndCommands() const {
        return m_shortcutControlsAndCommands;
    }

    /// Append/remove shortcuts hint when shortcuts are toggled
    void toggleKeyboardShortcutHints(bool enabled) {
        if (m_showKeyboardShortcuts == enabled) {
            return;
        }
        m_showKeyboardShortcuts = enabled;
        updateBaseTooltipOptShortcuts();
    }

    void updateBaseTooltipOptShortcuts();

  protected:
    // Whenever a connected control is changed, onConnectedControlChanged is
    // called. This allows the widget implementer to respond to the change and
    // gives them both the parameter and its corresponding value.
    virtual void onConnectedControlChanged(double dParameter, double dValue) {
        Q_UNUSED(dParameter);
        Q_UNUSED(dValue);
    }

    void resetControlParameter();
    void setControlParameter(double v);
    void setControlParameterDown(double v);
    void setControlParameterUp(double v);
    void setControlParameterLeftDown(double v);
    void setControlParameterLeftUp(double v);
    void setControlParameterRightDown(double v);
    void setControlParameterRightUp(double v);

    // Tooltip handling. We support "debug tooltips" which are basically a way
    // to expose debug information about widgets via the tooltip. To enable
    // this, when widgets should call updateTooltip before they are about to
    // display a tooltip.
    void updateTooltip();
    virtual void fillDebugTooltip(QStringList* debug);

    std::vector<std::unique_ptr<ControlParameterWidgetConnection>> m_connections;
    ControlParameterWidgetConnection* m_pDisplayConnection;
    std::vector<std::unique_ptr<ControlParameterWidgetConnection>> m_leftConnections;
    std::vector<std::unique_ptr<ControlParameterWidgetConnection>> m_rightConnections;

    std::vector<std::unique_ptr<ControlWidgetPropertyConnection>> m_propertyConnections;

  private:
    QWidget* m_pWidget;

    QString m_baseTooltip;
    QString m_shortcutTooltip;
    QString m_baseTooltipOptShortcuts;
    // Map of [ConfigKey, tr string] of control or all sub-controls.
    QList<std::pair<ConfigKey, QString>> m_shortcutControlsAndCommands;

    bool m_showKeyboardShortcuts;

    friend class ControlParameterWidgetConnection;
};
