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
class WTrackTableView;

class DlgRecording : public QFrame, public Ui::DlgRecording {
    Q_OBJECT
  public:
    DlgRecording(QWidget *parent, TrackCollection* pTrackCollection,
                 RecordingManager* pRecManager);
    virtual ~DlgRecording();

    virtual void onShow();
    void setTrackTable(WTrackTableView* pTrackTableView);

  public slots:
    void toggleRecording(bool toggle);
    void slotRecordingEnabled(bool);
    void slotBytesRecorded(long);
    void refreshBrowseModel();
    void slotDurationRecorded(QString durationRecorded);
    
  private:
    void refreshLabel();
    
    TrackCollection* m_pTrackCollection;
    BrowseTableModel m_browseModel;
    ProxyTrackModel m_proxyModel;
    QString m_recordingDir;
    QHash<int, WTrackTableView*> m_TrackTableView;

    QString m_bytesRecordedStr;
    QString m_durationRecordedStr;

    RecordingManager* m_pRecordingManager;
};

#endif //DLGRECORDING_H
