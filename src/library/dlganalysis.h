#pragma once

#include <QButtonGroup>
#include <QItemSelection>

#include "preferences/usersettings.h"
#include "library/analysislibrarytablemodel.h"
#include "library/libraryview.h"
#include "library/ui_dlganalysis.h"
#include "analyzer/analyzerprogress.h"

class AnalysisLibraryTableModel;
class WAnalysisLibraryTableView;
class Library;
class WLibrary;

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
    void loadSelectedTrack() override;
    void loadSelectedTrackToGroup(const QString& group, bool play) override;
    void slotAddToAutoDJBottom() override;
    void slotAddToAutoDJTop() override;
    void slotAddToAutoDJReplace() override;
    void moveSelection(int delta) override;
    inline const QString currentSearch() {
        return m_pAnalysisLibraryTableModel->currentSearch();
    }

  public slots:
    void tableSelectionChanged(const QItemSelection& selected,
                               const QItemSelection& deselected);
    void selectAll();
    void analyze();
    void slotAnalysisActive(bool bActive);
    void onTrackAnalysisSchedulerProgress(AnalyzerProgress analyzerProgress, int finishedCount, int totalCount);
    void onTrackAnalysisSchedulerFinished();
    void showRecentSongs();
    void showAllSongs();
    void installEventFilter(QObject* pFilter);

  signals:
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, const QString& player);
    void analyzeTracks(const QList<TrackId>& trackIds);
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
