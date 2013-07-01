#ifndef DLGSELECTOR_H
#define DLGSELECTOR_H

#include <QItemSelection>
#include "ui_dlgselector.h"
#include "configobject.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "mixxxkeyboard.h"

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
    void setSeedTrack(TrackPointer pSeedTrack);
    void loadSelectedTrack();
    void loadSelectedTrackToGroup(QString group);
    void moveSelection(int delta);

  public slots:
    void tableSelectionChanged(const QItemSelection& selected,
                               const QItemSelection& deselected);
    void selectAll();
    void resetFilters();
    void filterByGenre();
    void filterByBpm();
    void spinBoxBpmRangeChanged(int value);
    void filterByKey();
    void filterByKey4th();
    void filterByKey5th();
    void filterByKeyRelative();
    void installEventFilter(QObject* pFilter);
    void slotFiltersChanged();
    void slotSeedTrackInfoChanged();

  signals:
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, QString player);

private slots:
    void on_buttonCalcSimilarity_clicked();

private:
    //Note m_pTrackTablePlaceholder is defined in the .ui file
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    //QButtonGroup m_songsButtonGroup;
    WTrackTableView* m_pTrackTableView;
    SelectorLibraryTableModel* m_pSelectorLibraryTableModel;
    QSqlTableModel* m_pCratesTableModel;
};

#endif //DLGSELECTOR_H
