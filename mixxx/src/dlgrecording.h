#ifndef DLGRECORDING_H
#define DLGRECORDING_H

#include <QItemSelection>
#include "ui_dlgrecording.h"
#include "configobject.h"
#include "trackinfoobject.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "library/browse/browsetablemodel.h"
#include "library/proxytrackmodel.h"
#include "recording/recordingmanager.h"
#include "mixxxkeyboard.h"

class PlaylistTableModel;
class WTrackTableView;
class AnalyserQueue;
class QSqlTableModel;
class ControlObjectThreadMain;

class DlgRecording : public QWidget, public Ui::DlgRecording, public virtual LibraryView {
    Q_OBJECT
  public:
    DlgRecording(QWidget *parent, ConfigObject<ConfigValue>* pConfig,
                 TrackCollection* pTrackCollection,
                 RecordingManager* pRecManager, MixxxKeyboard* pKeyboard);
    virtual ~DlgRecording();

    virtual void setup(QDomNode node);
    virtual void onSearchStarting();
    virtual void onSearchCleared();
    virtual void onSearch(const QString& text);
    virtual void onShow();
    virtual void loadSelectedTrack();
    virtual void loadSelectedTrackToGroup(QString group);
    virtual void moveSelection(int delta);
    void refreshBrowseModel();
    inline const QString currentSearch() { return m_proxyModel.currentSearch(); }

  public slots:
    void toggleRecording(bool toggle);
    void slotRecordingEnabled(bool);
    void slotBytesRecorded(long);

  signals:
    void loadTrack(TrackPointer tio);
    void loadTrackToPlayer(TrackPointer tio, QString group);

  private:

    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    WTrackTableView* m_pTrackTableView;
    BrowseTableModel m_browseModel;
    ProxyTrackModel m_proxyModel;
    QString m_recordingDir;

    RecordingManager* m_pRecordingManager;

};

#endif //DLGRECORDING_H


