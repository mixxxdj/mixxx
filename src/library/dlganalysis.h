#ifndef DLGANALYSIS_H
#define DLGANALYSIS_H

#include <QItemSelection>

#include "preferences/usersettings.h"
#include "library/analysislibrarytablemodel.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "library/ui_dlganalysis.h"

class AnalysisLibraryTableModel;
class WAnalysisLibraryTableView;

class DlgAnalysis : public QWidget, public Ui::DlgAnalysis {
    
    Q_OBJECT
    
  public:
    
    DlgAnalysis(QWidget *parent,
               TrackCollection* pTrackCollection);
    virtual ~DlgAnalysis();

    virtual void onShow();
    inline const QString currentSearch() {
        return m_pAnalysisLibraryTableModel->currentSearch();
    }
    int getNumTracks();
    
    void setAnalysisTableView(WAnalysisLibraryTableView* pTable);
    inline void setFocusedPane(int pane) {
    	m_focusedPane = pane;
    }
    void setSelectedIndexes(const QModelIndexList& selectedIndexes);

  public slots:
    
    void selectAll();
    void analyze();
    void trackAnalysisFinished(int size);
    void trackAnalysisProgress(int progress);
    void trackAnalysisStarted(int size);
    void showRecentSongs();
    void showAllSongs();
    void analysisActive(bool bActive);

  signals:
    
    void analyzeTracks(QList<TrackId> trackIds);
    void stopAnalysis();

  private:
    //Note m_pTrackTablePlaceholder is defined in the .ui file
    TrackCollection* m_pTrackCollection;
    bool m_bAnalysisActive;
    QButtonGroup m_songsButtonGroup;
    AnalysisLibraryTableModel* m_pAnalysisLibraryTableModel;
    int m_tracksInQueue;
    int m_currentTrack;
    
    QModelIndexList m_selectedIndexes;
};

#endif //DLGTRIAGE_H
