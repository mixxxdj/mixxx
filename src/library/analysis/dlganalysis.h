#pragma once

#include <QButtonGroup>

#include "analyzer/analyzerprogress.h"
#include "analyzer/analyzerscheduledtrack.h"
#include "library/analysis/analysislibrarytablemodel.h"
#include "library/analysis/ui_dlganalysis.h"
#include "library/libraryview.h"
#include "preferences/usersettings.h"

class Library;
class WAnalysisLibraryTableView;
class WLibrary;
class QItemSelection;

class DlgAnalysis : public QWidget, public Ui::DlgAnalysis, public virtual LibraryView {
    Q_OBJECT
  public:
    DlgAnalysis(WLibrary *parent,
               UserSettingsPointer pConfig,
               Library* pLibrary);
    ~DlgAnalysis() override = default;

    void onSearch(const QString& text) override;
    void onShow() override;
    bool hasFocus() const override;
    void setFocus() override;
    inline const QString currentSearch() {
        return m_pAnalysisLibraryTableModel->currentSearch();
    }
    void saveCurrentViewState() override;
    bool restoreCurrentViewState() override;

  public slots:
    void tableSelectionChanged(const QItemSelection& selected,
                               const QItemSelection& deselected);
    void selectAll();
    void analyze();
    void slotAnalysisActive(bool bActive);
    void onTrackAnalysisSchedulerProgress(AnalyzerProgress analyzerProgress, int finishedCount, int totalCount);
    void onTrackAnalysisSchedulerFinished();
    void slotShowRecentSongs();
    void slotShowAllSongs();
    void installEventFilter(QObject* pFilter);

  signals:
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, const QString& player);
    void analyzeTracks(const QList<AnalyzerScheduledTrack>& tracks);
    void stopAnalysis();
    void trackSelected(TrackPointer pTrack);

  private:
    //Note m_pTrackTablePlaceholder is defined in the .ui file
    UserSettingsPointer m_pConfig;
    bool m_bAnalysisActive;
    QButtonGroup m_songsButtonGroup;
    WAnalysisLibraryTableView* m_pAnalysisLibraryTableView;
    AnalysisLibraryTableModel* m_pAnalysisLibraryTableModel;
};
