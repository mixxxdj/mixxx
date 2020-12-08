#pragma once

#include <QItemSelection>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/library.h"
#include "library/libraryview.h"
#include "library/ui_dlgmissing.h"
#include "preferences/usersettings.h"
#include "widget/wtracktableview.h"

class WLibrary;
class MissingTableModel;

class DlgMissing : public QWidget, public Ui::DlgMissing, public LibraryView {
    Q_OBJECT

  public:
    DlgMissing(WLibrary* parent, UserSettingsPointer pConfig,
               Library* pLibrary,
               KeyboardEventFilter* pKeyboard);
    ~DlgMissing() override;

    void onShow() override;
    bool hasFocus() const override;
    void onSearch(const QString& text) override;
    QString currentSearch();
    void saveCurrentViewState() override {
        m_pTrackTableView->saveCurrentViewState();
    };
    void restoreCurrentViewState() override {
        m_pTrackTableView->restoreCurrentViewState();
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
    MissingTableModel* m_pMissingTableModel;
};
