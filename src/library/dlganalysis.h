#ifndef DLGANALYSIS_H
#define DLGANALYSIS_H

#include <QItemSelection>
#include <QButtonGroup>

#include "preferences/usersettings.h"
#include "library/analysislibrarytablemodel.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "library/ui_dlganalysis.h"

class AnalysisLibraryTableModel;
class WAnalysisLibraryTableView;
class AnalysisFeature;

class DlgAnalysis : public QFrame, public Ui::DlgAnalysis {
    
    Q_OBJECT
    
  public:
    
    DlgAnalysis(QWidget *parent,
                AnalysisFeature* pAnalysis,
                TrackCollection* pTrackCollection);
    virtual ~DlgAnalysis();

    virtual void onShow();
    inline const QString currentSearch() {
        return m_pAnalysisLibraryTableModel->currentSearch();
    }
    int getNumTracks();
    
    // The selected indexes are always from the focused pane
    void setSelectedIndexes(const QModelIndexList& selectedIndexes);
    void setTableModel(AnalysisLibraryTableModel* pTableModel);

  public slots:
    
    void analyze();
    void trackAnalysisFinished(int size);
    void trackAnalysisProgress(int progress);
    void trackAnalysisStarted(int size);
    void analysisActive(bool bActive);
    
  private:
    //Note m_pTrackTablePlaceholder is defined in the .ui file
    TrackCollection* m_pTrackCollection;
    bool m_bAnalysisActive;
    QButtonGroup m_songsButtonGroup;
    AnalysisLibraryTableModel* m_pAnalysisLibraryTableModel;
    AnalysisFeature* m_pAnalysis;
    int m_tracksInQueue;
    int m_currentTrack;
    
    QModelIndexList m_selectedIndexes;
};

#endif //DLGTRIAGE_H
