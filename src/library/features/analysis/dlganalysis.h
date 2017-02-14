#ifndef DLGANALYSIS_H
#define DLGANALYSIS_H

#include <QButtonGroup>
#include <QPointer>

#include "library/features/analysis/analysislibrarytablemodel.h"
#include "library/features/analysis/analysisfeature.h"
#include "library/features/analysis/ui_dlganalysis.h"

class AnalysisFeature;

class DlgAnalysis : public QFrame, public Ui::DlgAnalysis {
    
    Q_OBJECT
    
  public:
    
    DlgAnalysis(QWidget* parent, AnalysisFeature* pAnalysis);
    ~DlgAnalysis() override;

    void onShow();
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
    bool m_bAnalysisActive;
    QButtonGroup m_songsButtonGroup;
    QPointer<AnalysisLibraryTableModel> m_pAnalysisLibraryTableModel;
    QPointer<AnalysisFeature> m_pAnalysis;
    int m_tracksInQueue;
    int m_currentTrack;
    
    QModelIndexList m_selectedIndexes;
};

#endif //DLGTRIAGE_H
