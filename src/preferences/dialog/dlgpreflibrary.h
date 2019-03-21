#ifndef DLGPREFLIBRARY_H
#define DLGPREFLIBRARY_H

#include <QStandardItemModel>
#include <QWidget>
#include <QFont>

#include "preferences/dialog/ui_dlgpreflibrarydlg.h"
#include "preferences/usersettings.h"
#include "library/library.h"
#include "library/library_preferences.h"
#include "preferences/dlgpreferencepage.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPrefLibrary : public DlgPreferencePage, public Ui::DlgPrefLibraryDlg  {
    Q_OBJECT
  public:
    enum TrackDoubleClickAction {
        LOAD_TO_DECK,
        ADD_TO_AUTODJ_BOTTOM,
        ADD_TO_AUTODJ_TOP
    };

    DlgPrefLibrary(
            QWidget* pParent,
            UserSettingsPointer pConfig,
            Library* pLibrary);
    ~DlgPrefLibrary() override {}

  public slots:
    // Common preference page slots.
    void slotUpdate();
    void slotShow();
    void slotHide();
    void slotResetToDefaults();
    void slotApply();
    void slotCancel();

    // Dialog to browse for music file directory
    void slotAddDir();
    void slotRemoveDir();
    void slotRelocateDir();

  signals:
    void apply();
    void scanLibrary();
    void requestAddDir(QString dir);
    void requestRemoveDir(QString dir, Library::RemovalType removalType);
    void requestRelocateDir(QString currentDir, QString newDir);

  private slots:
    void slotRowHeightValueChanged(int);
    void slotSelectFont();
    void slotSyncTrackMetadataExportToggled();
    void slotSearchDebouncingTimeoutMillisChanged(int);

  private:
    void initializeDirList();
    void setLibraryFont(const QFont& font);

    QStandardItemModel m_dirListModel;
    UserSettingsPointer m_pConfig;
    Library* m_pLibrary;
    bool m_bAddedDirectory;
    QFont m_originalTrackTableFont;
    int m_iOriginalTrackTableRowHeight;
};

#endif
