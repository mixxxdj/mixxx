#pragma once

#include <QDomNode>
#include <QEvent>
#include <QLineEdit>
#include <QTimer>
#include <QToolButton>

#include "util/parented_ptr.h"
#include "widget/wbasewidget.h"

class SkinContext;

class WSearchLineEdit : public QLineEdit, public WBaseWidget {
    Q_OBJECT
  public:
    // Delay for triggering a search while typing
    static constexpr int kMinDebouncingTimeoutMillis = 100;
    static constexpr int kDefaultDebouncingTimeoutMillis = 300;
    static constexpr int kMaxDebouncingTimeoutMillis = 9999;

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
    void slotSetFont(const QFont& font);

    void slotRestoreSearch(const QString& text);
    void slotDisableSearch();

    void slotClearSearch();
    bool slotClearSearchIfClearButtonHasFocus();

  private slots:
    void slotSetShortcutFocus();
    void slotTextChanged(const QString& text);

    void slotTriggerSearch();

  private:
    // TODO(XXX): This setting shouldn't be static and the widget
    // should instead define a public slot for changing the value.
    // But this would require to connect the widget instance to some
    // value provider that sends signals whenever the corresponding
    // configuration value changes.
    static int s_debouncingTimeoutMillis;

    void refreshState();

    void enableSearch(const QString& text);
    void updateEditBox(const QString& text);
    void updateClearButton(const QString& text);

    QString getSearchText() const;

    // Update the displayed text without (re-)starting the timer
    void setTextBlockSignals(const QString& text);

    parented_ptr<QToolButton> const m_clearButton;

    int m_frameWidth;
    int m_innerHeight;

    QTimer m_debouncingTimer;
};
