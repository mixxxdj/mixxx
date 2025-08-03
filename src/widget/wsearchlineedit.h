#pragma once

#include <QComboBox>
#include <QTimer>

#include "library/library_decl.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"
#include "widget/wbasewidget.h"

// EVE
#include "library/basesqltablemodel.h"
#include "library/library.h"
#include "library/trackset/searchcrate/searchcrateid.h"
#include "library/trackset/tracksettablemodel.h"

// EVE

class QDomNode;
class SkinContext;
class QCompleter;
class QEvent;
class QToolButton;

class WSearchLineEdit : public QComboBox, public WBaseWidget {
    Q_OBJECT
  public:
    // Delay for triggering a search while typing
    static constexpr int kMinDebouncingTimeoutMillis = 100;
    static constexpr int kDefaultDebouncingTimeoutMillis = 300;
    static constexpr int kMaxDebouncingTimeoutMillis = 9999;
    static constexpr int kSaveTimeoutMillis = 5000;
    static constexpr int kMaxSearchEntries = 50;
    static constexpr bool kCompletionsEnabledDefault = true;
    static constexpr bool kHistoryShortcutsEnabledDefault = true;

    // TODO(XXX): Replace with a public slot
    static void setDebouncingTimeoutMillis(int debouncingTimeoutMillis);
    static void setSearchCompletionsEnabled(bool searchCompletionsEnabled);
    static void setSearchHistoryShortcutsEnabled(bool searchHistoryShortcutsEnabled);
    virtual void showPopup() override;

    void showFastSearchDialog();
    void handleSearch(const QString& query);
    void handleSearchToCrate(const QString& query);
    void slotShowFastSearchDialog();

    explicit WSearchLineEdit(QWidget* pParent, UserSettingsPointer pConfig = nullptr);
    ~WSearchLineEdit();

    void setup(const QDomNode& node, const SkinContext& context);
    void setupToolTip(const QString& searchInCurrentViewShortcut,
            const QString& searchInAllTracksShortcut);

    void setFocus(Qt::FocusReason focusReason);

  protected:
    void resizeEvent(QResizeEvent*) override;
    void focusInEvent(QFocusEvent*) override;
    void focusOutEvent(QFocusEvent*) override;
    bool event(QEvent*) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

  signals:
    void search(const QString& text);
    void newSearchCrate(const QString& text);
    FocusWidget setLibraryFocus(FocusWidget newFocusWidget,
            Qt::FocusReason focusReason = Qt::OtherFocusReason);

  public slots:
    void slotSetFont(const QFont& font);

    void slotRestoreSearch(const QString& text);
    void slotDisableSearch();

    void slotClearSearch();
    bool slotClearSearchIfClearButtonHasFocus();
    // EVE
    void slot2SearchCrate();
    bool slot2SearchCrateIf2SearchCrateButtonHasFocus();
    // EVE

    /// The function selects an entry relative to the currently selected
    /// entry in the history and executes the search.
    /// The parameter specifies the distance in steps (positive/negative = downward/upward)
    void slotMoveSelectedHistory(int steps);
    void slotDeleteCurrentItem();

  private slots:
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
    static bool s_completionsEnabled;
    static bool s_historyShortcutsEnabled;

    void refreshState();

    void enableSearch(const QString& text);
    void updateEditBox(const QString& text);
    void updateClearAndDropdownButton(const QString& text);
    void updateCompleter();
    void deleteSelectedComboboxItem();
    void deleteSelectedListItem();
    void triggerSearchDebounced();
    bool hasSelectedText() const;
    bool hasCompletionAvailable(QString* completionPrefix = nullptr) const;

    inline int findCurrentTextIndex() {
        return findData(currentText().trimmed(), Qt::DisplayRole);
    }

    QString getSearchText() const;

    // Update the displayed text without (re-)starting the timer
    void setTextBlockSignals(const QString& text);

    UserSettingsPointer m_pConfig;
    void loadQueriesFromConfig();
    void saveQueriesInConfig();

    parented_ptr<QCompleter> m_completer;
    parented_ptr<QToolButton> const m_clearButton;
    // EVE
    parented_ptr<QToolButton> const m_2SearchCrateButton;
    // EVE
    QTimer m_debouncingTimer;
    QTimer m_saveTimer;
    bool m_queryEmitted;
    parented_ptr<QAction> m_pFastSearchAction;
};
