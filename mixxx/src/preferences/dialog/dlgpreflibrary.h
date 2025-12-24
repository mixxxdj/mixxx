#pragma once

#include <QFont>
#include <QStandardItemModel>
#include <memory>

#include "library/library_decl.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgpreflibrarydlg.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

class QWidget;
class ControlProxy;

class DlgPrefLibrary : public DlgPreferencePage, public Ui::DlgPrefLibraryDlg  {
    Q_OBJECT
  public:
    enum class TrackDoubleClickAction : int {
        LoadToDeck = 0,
        AddToAutoDJBottom = 1,
        AddToAutoDJTop = 2,
        Ignore = 3,
    };

    enum class CoverArtFetcherQuality {
        Low = 0,
        Medium = 1,
        High = 2,
        Highest = 3,
    };

    DlgPrefLibrary(
            QWidget* pParent,
            UserSettingsPointer pConfig,
            std::shared_ptr<Library> pLibrary);
    ~DlgPrefLibrary() override;

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
    void requestRemoveDir(const QString& dir, LibraryRemovalType removalType);
    void requestRelocateDir(const QString& currentDir, const QString& newDir);

  private slots:
    void slotRowHeightValueChanged(int);
    void slotSelectFont();
    void slotSyncTrackMetadataToggled();
    void slotSearchDebouncingTimeoutMillisChanged(int);
    void slotBpmRangeSelected(int index);
    void slotBpmColumnPrecisionChanged(int bpmPrecision);
    void slotSeratoMetadataExportClicked(bool);

  private:
    void populateDirList();
    void setLibraryFont(const QFont& font);
    void resetLibraryFont();
    void updateSearchLineEditHistoryOptions();
    void setSeratoMetadataEnabled(bool shouldSyncTrackMetadata);

    QStandardItemModel m_dirListModel;
    UserSettingsPointer m_pConfig;
    std::shared_ptr<Library> m_pLibrary;
    bool m_bAddedDirectory;
    QFont m_originalTrackTableFont;
    int m_iOriginalTrackTableRowHeight;
    // Listen to rate range changes in order to update the fuzzy BPM range
    parented_ptr<ControlProxy> m_pRateRangeDeck1;
};
