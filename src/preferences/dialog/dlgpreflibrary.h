#ifndef DLGPREFLIBRARY_H
#define DLGPREFLIBRARY_H

#include <QFont>
#include <QStandardItemModel>
#include <QWidget>

#include "defs_urls.h"
#include "library/library.h"
#include "library/library_preferences.h"
#include "preferences/dialog/ui_dlgpreflibrarydlg.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/usersettings.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPrefLibrary : public DlgPreferencePage, public Ui::DlgPrefLibraryDlg  {
    Q_OBJECT
  public:
    enum class TrackDoubleClickAction : int {
        LoadToDeck = 0,
        AddToAutoDJBottom = 1,
        AddToAutoDJTop = 2,
        Ignore = 3,
    };

    DlgPrefLibrary(
            QWidget* pParent,
            UserSettingsPointer pConfig,
            Library* pLibrary);
    ~DlgPrefLibrary() override {}

    QUrl helpUrl() const override;

  public slots:
    // Common preference page slots.
    void slotUpdate() override;
    void slotShow() override;
    void slotHide() override;
    void slotResetToDefaults() override;
    void slotApply() override;
    void slotCancel() override;

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
