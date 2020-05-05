#pragma once

#include <QDomNode>
#include <QEvent>
#include <QLineEdit>
#include <QTimer>
#include <QToolButton>

#include "widget/wbasewidget.h"

class SkinContext;

class WSearchLineEdit : public QLineEdit, public WBaseWidget {
    Q_OBJECT
  public:
    enum class State {
        Active,
        Inactive,
    };

    // Delay for triggering a search while typing
    static constexpr int kMinDebouncingTimeoutMillis = 100;
    static constexpr int kDefaultDebouncingTimeoutMillis = 300;
    static constexpr int kMaxDebouncingTimeoutMillis = 9999;

    // Use 'active' property to apply an alternative style to the
    // placeholder text
    Q_PROPERTY(bool active READ isActive);

    bool isActive() const {
        return m_state == State::Active ? true : false;
    }

    // TODO(XXX): Replace with a public slot
    static void setDebouncingTimeoutMillis(int debouncingTimeoutMillis);

    explicit WSearchLineEdit(QWidget* pParent);
    ~WSearchLineEdit() override = default;

    void setup(const QDomNode& node, const SkinContext& context);

  protected:
    void resizeEvent(QResizeEvent*) override;
    void focusInEvent(QFocusEvent*) override;
    void focusOutEvent(QFocusEvent*) override;
    bool event(QEvent*) override;

  signals:
    void search(const QString& text);

  public slots:
    void restoreSearch(const QString& text);
    void disableSearch();
    void slotSetFont(const QFont& font);

    void clearSearch();
    bool clearSearchIfClearButtonHasFocus();

  private slots:
    void setShortcutFocus();
    void updateText(const QString& text);

    void triggerSearch();

  private:
    // TODO(XXX): This setting shouldn't be static and the widget
    // should instead define a public slot for changing the value.
    // But this would require to connect the widget instance to some
    // value provider that sends signals whenever the corresponding
    // configuration value changes.
    static int s_debouncingTimeoutMillis;

    void switchState(State state);
    void updateEditBox(const QString& text);
    void refreshEditBox();

    void showPlaceholder();
    bool shouldShowPlaceholder(const QString& text) const;

    void enableSearch(const QString& text);
    void showSearchText(const QString& text);

    void updateClearButton(const QString& text);

    QString getSearchText() const;

    // Update the displayed text without (re-)starting the timer
    void setTextBlockSignals(const QString& text);

    QToolButton* const m_clearButton;

    int m_frameWidth;
    int m_innerHeight;

    QTimer m_debouncingTimer;

    State m_state;
};
