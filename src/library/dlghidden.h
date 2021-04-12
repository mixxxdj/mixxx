#pragma once

#include <QItemSelection>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/library.h"
#include "library/libraryview.h"
#include "library/ui_dlghidden.h"
#include "preferences/usersettings.h"
#include "widget/wtracktableview.h"

class WLibrary;
class HiddenTableModel;

class DlgHidden : public QWidget, public Ui::DlgHidden, public LibraryView {
    Q_OBJECT

  public:
    DlgHidden(WLibrary* parent, UserSettingsPointer pConfig,
              Library* pLibrary,
              KeyboardEventFilter* pKeyboard);
    ~DlgHidden() override;

    void onShow() override;
    bool hasFocus() const override;
    void onSearch(const QString& text) override;
    QString currentSearch();
    void saveCurrentViewState() override {
        m_pTrackTableView->saveCurrentViewState();
    };
    bool restoreCurrentViewState(bool fromSearch = false) override {
        return m_pTrackTableView->restoreCurrentViewState(fromSearch);
    };

  public slots:
    void clicked();
    void selectAll();
    void selectionChanged(const QItemSelection&, const QItemSelection&);

  signals:
    void trackSelected(TrackPointer pTrack);

  private:
    void activateButtons(bool enable);
    WTrackTableView* m_pTrackTableView;
    HiddenTableModel* m_pHiddenTableModel;
};
