#ifndef DLGRECORDING_H
#define DLGRECORDING_H

#include "configobject.h"
#include "library/browse/browsetablemodel.h"
#include "library/libraryview.h"
#include "library/proxytrackmodel.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "mixxxkeyboard.h"
#include "recording/recordingmanager.h"
#include "trackinfoobject.h"
#include "ui_dlgrecording.h"

class AnalyserQueue;
class PlaylistTableModel;
class QSqlTableModel;
class WTrackTableView;

class DlgRecording : public QWidget, public Ui::DlgRecording, public virtual LibraryView {
    Q_OBJECT
  public:
    DlgRecording(QWidget *parent, ConfigObject<ConfigValue>* pConfig,
                 Library* pLibrary, TrackCollection* pTrackCollection,
                 RecordingManager* pRecManager, MixxxKeyboard* pKeyboard);
    virtual ~DlgRecording();

    virtual void onSearch(const QString& text);
    virtual void onShow();
    virtual void loadSelectedTrack();
    virtual void slotSendToAutoDJ();
    virtual void slotSendToAutoDJTop();
    virtual void loadSelectedTrackToGroup(QString group, bool play);
    virtual void moveSelection(int delta);
    inline const QString currentSearch() { return m_proxyModel.currentSearch(); }

  public slots:
    void toggleRecording(bool toggle);
    void slotRecordingEnabled(bool);
    void slotBytesRecorded(long);
    void refreshBrowseModel();
    void slotRestoreSearch();
    void slotDurationRecorded(QString durationRecorded);
    void setTrackTableFont(const QFont& font);
    void setTrackTableRowHeight(int rowHeight);

  signals:
    void loadTrack(TrackPointer tio);
    void loadTrackToPlayer(TrackPointer tio, QString group, bool play);
    void restoreSearch(QString search);

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    WTrackTableView* m_pTrackTableView;
    BrowseTableModel m_browseModel;
    ProxyTrackModel m_proxyModel;
    QString m_recordingDir;

    void refreshLabel();
    QString m_bytesRecordedStr;
    QString m_durationRecordedStr;

    RecordingManager* m_pRecordingManager;
};

#endif //DLGRECORDING_H
