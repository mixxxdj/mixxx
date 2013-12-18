#ifndef DLGSELECTOR_H
#define DLGSELECTOR_H

#include <QItemSelection>

#include "configobject.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "library/selector/selectorfilters.h"
#include "mixxxkeyboard.h"
#include "ui_dlgselector.h"

class SelectorLibraryTableModel;
class WTrackTableView;
class QSqlTableModel;

class DlgSelector : public QWidget, public Ui::DlgSelector, public LibraryView {
    Q_OBJECT
  public:
    DlgSelector(QWidget *parent,
               ConfigObject<ConfigValue>* pConfig,
               TrackCollection* pTrackCollection,
               MixxxKeyboard* pKeyboard);
    ~DlgSelector();

    void onShow();
    void onHide();
    void onSearch(const QString &text);
    void loadSelectedTrack();
    void loadSelectedTrackToGroup(QString group);
    void moveSelection(int delta);

  public slots:
    void filterByGenre(bool checked);
    void filterByBpm(bool checked);
    void bpmRangeChanged(int value);
    void filterByKey(bool checked);
    void filterByKey4th(bool checked);
    void filterByKey5th(bool checked);
    void filterByKeyRelative(bool checked);
    void installEventFilter(QObject* pFilter);
    void setSeedTrack(TrackPointer pSeedTrack);
    void slotSeedTrackInfoChanged();
    void slotFiltersChanged();
    void calculateSimilarity();
    void calculateAllSimilarities(const QString& filename);

  signals:
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, QString player);

  private slots:
    void clearFilters();
    void loadStoredFilterSettings();
    void applyFilters();

  private:
    //Note m_pTrackTablePlaceholder is defined in the .ui file
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    WTrackTableView* m_pTrackTableView;
    SelectorLibraryTableModel* m_pSelectorLibraryTableModel;
    SelectorFilters* m_pSelectorFilters;
    QSqlTableModel* m_pCratesTableModel;
};

#endif //DLGSELECTOR_H
