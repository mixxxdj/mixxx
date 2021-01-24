#pragma once

#include <QFont>
#include <QStandardItemModel>
#include <QWidget>

#include "defs_urls.h"
#include "library/library.h"
#include "library/library_preferences.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgpreflibrarydlg.h"
#include "preferences/usersettings.h"

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
            std::shared_ptr<Library> pLibrary);
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
    void requestAddDir(const QString& dir);
    void requestRemoveDir(const QString& dir, Library::RemovalType removalType);
    void requestRelocateDir(const QString& currentDir, const QString& newDir);

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
    std::shared_ptr<Library> m_pLibrary;
    bool m_bAddedDirectory;
    QFont m_originalTrackTableFont;
    int m_iOriginalTrackTableRowHeight;
};
