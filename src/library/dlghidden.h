#pragma once

#include <QItemSelection>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/library.h"
#include "library/libraryview.h"
#include "library/ui_dlghidden.h"
#include "preferences/configobject.h"
#include "preferences/keyboardconfig.h"
#include "preferences/usersettings.h"

class WLibrary;
class WTrackTableView;
class HiddenTableModel;

class DlgHidden : public QWidget, public Ui::DlgHidden, public LibraryView {
    Q_OBJECT

  public:
    DlgHidden(WLibrary* parent,
            UserSettingsPointer pConfig,
            KeyboardConfigPointer pKbdConfig,
            Library* pLibrary,
            KeyboardEventFilter* pKeyboard);
    ~DlgHidden() override;

    void onShow() override;
    bool hasFocus() const override;
    void setFocus() override;
    void onSearch(const QString& text) override;
    QString currentSearch();
    void saveCurrentViewState() override;
    bool restoreCurrentViewState() override;

  public slots:
    void selectAll();
    void selectionChanged(const QItemSelection&, const QItemSelection&);

  signals:
    void trackSelected(TrackPointer pTrack);

  private:
    void activateButtons(bool enable);
    WTrackTableView* m_pTrackTableView;
    HiddenTableModel* m_pHiddenTableModel;
};
