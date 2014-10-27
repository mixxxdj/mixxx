#ifndef DLGANALYSIS_H
#define DLGANALYSIS_H

#include <QItemSelection>
#include "ui_dlganalysis.h"
#include "configobject.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "library/analysislibrarytablemodel.h"

class AnalysisLibraryTableModel;
class WAnalysisLibraryTableView;

class DlgAnalysis : public QWidget, public Ui::DlgAnalysis, public virtual LibraryView {
    Q_OBJECT
  public:
    DlgAnalysis(QWidget *parent,
               ConfigObject<ConfigValue>* pConfig,
               TrackCollection* pTrackCollection);
    virtual ~DlgAnalysis();

    virtual void onSearch(const QString& text);
    virtual void onShow();
    virtual void loadSelectedTrack();
    virtual void loadSelectedTrackToGroup(QString group, bool play);
    virtual void moveSelection(int delta);
    inline const QString currentSearch() {
        return m_pAnalysisLibraryTableModel->currentSearch();
    }
    int getNumTracks();

  public slots:
    void tableSelectionChanged(const QItemSelection& selected,
                               const QItemSelection& deselected);
    void selectAll();
    void analyze();
    void trackAnalysisFinished(int size);
    void trackAnalysisProgress(int progress);
    void trackAnalysisStarted(int size);
    void showRecentSongs();
    void showAllSongs();
    void installEventFilter(QObject* pFilter);
    void analysisActive(bool bActive);

  signals:
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, QString player);
    void analyzeTracks(QList<int> trackIds);
    void stopAnalysis();
    void trackSelected(TrackPointer pTrack);

  private:
    //Note m_pTrackTablePlaceholder is defined in the .ui file
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    bool m_bAnalysisActive;
    QButtonGroup m_songsButtonGroup;
    WAnalysisLibraryTableView* m_pAnalysisLibraryTableView;
    AnalysisLibraryTableModel* m_pAnalysisLibraryTableModel;
    int m_tracksInQueue;
    int m_currentTrack;
};

#endif //DLGTRIAGE_H
