#ifndef DLGRECORDING_H
#define DLGRECORDING_H

#include "preferences/usersettings.h"
#include "library/browse/browsetablemodel.h"
#include "library/libraryview.h"
#include "library/proxytrackmodel.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "recording/recordingmanager.h"
#include "track/track.h"
#include "library/recording/ui_dlgrecording.h"

class PlaylistTableModel;
class QSqlTableModel;
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
    void onShow() override;
    bool hasFocus() const override;
    void loadSelectedTrack() override;
    void slotAddToAutoDJBottom() override;
    void slotAddToAutoDJTop() override;
    void slotAddToAutoDJReplace() override;
    void loadSelectedTrackToGroup(QString group, bool play) override;
    void moveSelection(int delta) override;
    inline const QString currentSearch() { return m_proxyModel.currentSearch(); }

  public slots:
    void slotRecordingStateChanged(bool);
    void slotBytesRecorded(int);
    void refreshBrowseModel();
    void slotRestoreSearch();
    void slotDurationRecorded(QString durationRecorded);

  signals:
    void loadTrack(TrackPointer tio);
    void loadTrackToPlayer(TrackPointer tio, QString group, bool play);
    void restoreSearch(QString search);

  private:
    UserSettingsPointer m_pConfig;
    WTrackTableView* m_pTrackTableView;
    BrowseTableModel m_browseModel;
    ProxyTrackModel m_proxyModel;
    QString m_recordingDir;

    void refreshLabels();
    void slotRecButtonClicked(bool checked);
    QString m_bytesRecordedStr;
    QString m_durationRecordedStr;

    RecordingManager* m_pRecordingManager;
};

#endif //DLGRECORDING_H
