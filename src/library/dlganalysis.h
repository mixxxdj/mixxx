#ifndef DLGANALYSIS_H
#define DLGANALYSIS_H

#include <QButtonGroup>
#include <QItemSelection>

#include "preferences/usersettings.h"
#include "library/analysislibrarytablemodel.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "library/ui_dlganalysis.h"
#include "analyzer/analyzerprogress.h"

class AnalysisLibraryTableModel;
class WAnalysisLibraryTableView;
class Library;

class DlgAnalysis : public QWidget, public Ui::DlgAnalysis, public virtual LibraryView {
    Q_OBJECT
  public:
    DlgAnalysis(QWidget *parent,
               UserSettingsPointer pConfig,
               Library* pLibrary);
    ~DlgAnalysis() override = default;

    void onSearch(const QString& text) override;
    void onShow() override;
    bool hasFocus() const override;
    void loadSelectedTrack() override;
    void loadSelectedTrackToGroup(QString group, bool play) override;
    void slotSendToAutoDJBottom() override;
    void slotSendToAutoDJTop() override;
    void slotSendToAutoDJReplace() override;
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
    void loadTrackToPlayer(TrackPointer pTrack, QString player);
    void analyzeTracks(QList<TrackId> trackIds);
    void stopAnalysis();
    void trackSelected(TrackPointer pTrack);

  private:
    //Note m_pTrackTablePlaceholder is defined in the .ui file
    UserSettingsPointer m_pConfig;
    TrackCollection* m_pTrackCollection;
    bool m_bAnalysisActive;
    QButtonGroup m_songsButtonGroup;
    WAnalysisLibraryTableView* m_pAnalysisLibraryTableView;
    AnalysisLibraryTableModel* m_pAnalysisLibraryTableModel;
};

#endif //DLGTRIAGE_H
