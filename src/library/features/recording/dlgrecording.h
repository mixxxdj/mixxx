#ifndef DLGRECORDING_H
#define DLGRECORDING_H

#include "preferences/usersettings.h"
#include "library/features/browse/browsetablemodel.h"
#include "library/libraryview.h"
#include "library/proxytrackmodel.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "recording/recordingmanager.h"
#include "track/track.h"
#include "library/features/recording/ui_dlgrecording.h"

class PlaylistTableModel;
class QSqlTableModel;
class WTrackTableView;

class DlgRecording : public QFrame, public Ui::DlgRecording {
    Q_OBJECT
  public:
    DlgRecording(QWidget *parent, TrackCollection* pTrackCollection,
                 RecordingManager* pRecManager);
    ~DlgRecording() override;

    void onShow();
    void setProxyTrackModel(ProxyTrackModel* pProxyModel);
    void setBrowseTableModel(BrowseTableModel* pBrowseModel);

  public slots:
    void toggleRecording(bool toggle);
    void slotRecordingEnabled(bool);
    void slotBytesRecorded(int);
    void refreshBrowseModel();
    void slotDurationRecorded(QString durationRecorded);

  private:
    void refreshLabel();

    TrackCollection* m_pTrackCollection;
    BrowseTableModel* m_pBrowseModel;
    ProxyTrackModel* m_pProxyModel;
    QString m_recordingDir;

    QString m_bytesRecordedStr;
    QString m_durationRecordedStr;

    RecordingManager* m_pRecordingManager;
};

#endif //DLGRECORDING_H
