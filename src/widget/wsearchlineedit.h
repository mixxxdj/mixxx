#pragma once

#include <QComboBox>
#include <QDomNode>
#include <QEvent>
#include <QTimer>
#include <QToolButton>

#include "util/parented_ptr.h"
#include "widget/wbasewidget.h"

class SkinContext;

class WSearchLineEdit : public QComboBox, public WBaseWidget {
    Q_OBJECT
  public:
    // Delay for triggering a search while typing
    static constexpr int kMinDebouncingTimeoutMillis = 100;
    static constexpr int kDefaultDebouncingTimeoutMillis = 300;
    static constexpr int kMaxDebouncingTimeoutMillis = 9999;
    static constexpr int kSaveTimeoutMillis = 5000;
    static constexpr int kMaxSearchEntries = 50;

    // TODO(XXX): Replace with a public slot
    static void setDebouncingTimeoutMillis(int debouncingTimeoutMillis);
    virtual void showPopup() override;

    explicit WSearchLineEdit(QWidget* pParent);
    ~WSearchLineEdit() override = default;

    void setup(const QDomNode& node, const SkinContext& context);

  protected:
    void resizeEvent(QResizeEvent*) override;
    void focusInEvent(QFocusEvent*) override;
    void focusOutEvent(QFocusEvent*) override;
    bool event(QEvent*) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

  signals:
    void search(const QString& text);

  public slots:
    void slotSetFont(const QFont& font);

    void slotRestoreSearch(const QString& text);
    void slotDisableSearch();

    void slotClearSearch();
    bool slotClearSearchIfClearButtonHasFocus();

    /// The function selects an entry relative to the currently selected
    /// entry in the history and executes the search.
    /// The parameter specifies the distance in steps (positive/negative = downward/upward)
    void slotMoveSelectedHistory(int steps);

  private slots:
    void slotSetShortcutFocus();
    void slotTextChanged(const QString& text);
    void slotIndexChanged(int index);

    void slotTriggerSearch();
    void slotSaveSearch();

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
    void updateStyleMetrics();

    inline int findCurrentTextIndex() {
        return findData(currentText(), Qt::DisplayRole);
    }

    QString getSearchText() const;

    // Update the displayed text without (re-)starting the timer
    void setTextBlockSignals(const QString& text);

    parented_ptr<QToolButton> const m_clearButton;

    int m_frameWidth;
    int m_innerHeight;
    int m_dropButtonWidth;

    QTimer m_debouncingTimer;
    QTimer m_saveTimer;
};
