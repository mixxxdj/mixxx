#pragma once

#include "library/browse/browsetablemodel.h"
#include "library/libraryview.h"
#include "library/proxytrackmodel.h"
#include "library/recording/ui_dlgrecording.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

class WLibrary;
class WTrackTableView;
class Library;
class KeyboardEventFilter;

class DlgRecording : public QWidget, public Ui::DlgRecording, public virtual LibraryView {
    Q_OBJECT
  public:
    DlgRecording(WLibrary *parent, UserSettingsPointer pConfig,
                 Library* pLibrary,
                 RecordingManager* pRecManager, KeyboardEventFilter* pKeyboard);
    ~DlgRecording() override;

    void onSearch(const QString& text) override;
    void onShow() override{};
    bool hasFocus() const override;
    void setFocus() override;
    inline const QString currentSearch() { return m_proxyModel.currentSearch(); }
    void saveCurrentViewState() override;
    bool restoreCurrentViewState() override;

  public slots:
    void slotRecordingStateChanged(bool);
    void slotBytesRecorded(int);
    void refreshBrowseModel();
    void slotRestoreSearch();
    void slotDurationRecorded(const QString& durationRecorded);

  signals:
    void loadTrack(TrackPointer tio);
    void loadTrackToPlayer(TrackPointer tio, const QString& group, bool play);
    void restoreSearch(const QString& search);
    void restoreModelState();

  private:
    UserSettingsPointer m_pConfig;
    WTrackTableView* m_pTrackTableView;
    BrowseTableModel m_browseModel;
    ProxyTrackModel m_proxyModel;

    void refreshLabels();
    void slotRecButtonClicked(bool checked);
    QString m_bytesRecordedStr;
    QString m_durationRecordedStr;

    RecordingManager* m_pRecordingManager;
};
