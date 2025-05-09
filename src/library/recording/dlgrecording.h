#pragma once

#include "library/libraryview.h"
#include "library/recording/ui_dlgrecording.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#ifdef __STEM__
#include "engine/engine.h"
#endif
#include "util/parented_ptr.h"

class BrowseTableModel;
class BrowseLibraryTableModel;
class KeyboardEventFilter;
class Library;
class ProxyTrackModel;
class RecordingManager;
class TrackCollection;
class TrackModel;
class WLibrary;
class WTrackTableView;

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
    QString currentSearch() const;
    void saveCurrentViewState() override;
    bool restoreCurrentViewState() override;

  public slots:
    void slotRecordingStateChanged(bool);
    void slotBytesRecorded(int);
    void refreshBrowseModel();
    void slotRestoreSearch();
    void slotDurationRecorded(const QString& durationRecorded);

    void slotLoadTrack(TrackPointer pTrack);

  signals:
    void loadTrack(TrackPointer tio);
#ifdef __STEM__
    void loadTrackToPlayer(TrackPointer tio,
            const QString& group,
            mixxx::StemChannelSelection stemMask,
            bool);
#else
    void loadTrackToPlayer(TrackPointer tio, const QString& group, bool);
#endif
    void restoreSearch(const QString& search);
    void restoreModelState();

  private:
    UserSettingsPointer m_pConfig;
    TrackCollection* const m_pTrackCollection;

    parented_ptr<WTrackTableView> m_pTrackTableView;
    parented_ptr<BrowseTableModel> m_pBrowseModel;
    ProxyTrackModel* m_pProxyModel;
    parented_ptr<BrowseLibraryTableModel> m_pLibraryTableModel;
    TrackModel* m_pCurrentTrackModel;

    void refreshLabels();
    void slotRecButtonClicked(bool checked);
    QString m_bytesRecordedStr;
    QString m_durationRecordedStr;

    TrackPointer m_currentRecTrack;

    RecordingManager* m_pRecordingManager;
};
