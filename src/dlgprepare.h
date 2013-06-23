#ifndef DLGTRIAGE_H
#define DLGTRIAGE_H

#include <QItemSelection>
#include "ui_dlgprepare.h"
#include "configobject.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "library/preparelibrarytablemodel.h"

class PrepareLibraryTableModel;
class WPrepareCratesTableView;
class WPrepareLibraryTableView;

class DlgPrepare : public QWidget, public Ui::DlgPrepare, public virtual LibraryView {
    Q_OBJECT
  public:
    DlgPrepare(QWidget *parent,
               ConfigObject<ConfigValue>* pConfig,
               TrackCollection* pTrackCollection);
    virtual ~DlgPrepare();

    virtual void onSearch(const QString& text);
    virtual void onShow();
    virtual void loadSelectedTrack();
    virtual void loadSelectedTrackToGroup(QString group, bool play);
    virtual void moveSelection(int delta);
    inline const QString currentSearch() { return m_pPrepareLibraryTableModel->currentSearch(); };

  public slots:
    void tableSelectionChanged(const QItemSelection& selected,
                               const QItemSelection& deselected);
    void selectAll();
    void analyze();
    void trackAnalysisFinished(int size);
    void trackAnalysisProgress(int progress);
    void showRecentSongs();
    void showAllSongs();
    void installEventFilter(QObject* pFilter);
    void analysisActive(bool bActive);

  signals:
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, QString player);
    void analyzeTracks(QList<int> trackIds);
    void stopAnalysis();

  private:
    //Note m_pTrackTablePlaceholder is defined in the .ui file
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    bool m_bAnalysisActive;
    QButtonGroup m_songsButtonGroup;
    WPrepareLibraryTableView* m_pPrepareLibraryTableView;
    PrepareLibraryTableModel* m_pPrepareLibraryTableModel;
    WPrepareCratesTableView* m_pPrepareCratesTableView;
    int m_tracksInQueue;
    int m_currentTrack;
};

#endif //DLGTRIAGE_H
