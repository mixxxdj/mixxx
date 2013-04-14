#ifndef DLGSELECTOR_H
#define DLGSELECTOR_H

#include <QItemSelection>
#include "ui_dlgselector.h"
#include "configobject.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"

class SelectorLibraryTableModel;
class WSelectorCratesTableView;
class WSelectorLibraryTableView;
class QSqlTableModel;
class CrateView;

class DlgSelector : public QWidget, public Ui::DlgSelector, public virtual LibraryView {
    Q_OBJECT
  public:
    DlgSelector(QWidget *parent,
               ConfigObject<ConfigValue>* pConfig,
               TrackCollection* pTrackCollection);
    virtual ~DlgSelector();

    virtual void setup(QDomNode node);
    virtual void onSearchStarting();
    virtual void onSearchCleared();
    virtual void onSearch(const QString& text);
    virtual void onShow();
    virtual void onHide();
    virtual void loadSelectedTrack();
    virtual void loadSelectedTrackToGroup(QString group, bool play);
    virtual void moveSelection(int delta);

  public slots:
    void tableSelectionChanged(const QItemSelection& selected,
                               const QItemSelection& deselected);
    void selectAll();
    void filterByGenre();
    void filterByBpm();
    void spinBoxBpmRangeChanged(int value);
    void filterByYear();
    void filterByRating();
    void filterByKey();
    void filterByKey4th();
    void filterByKey5th();
    void filterByKeyRelative();
    void installEventFilter(QObject* pFilter);
    void slotFiltersChanged();
    void slotCurrentTrackInfoChanged();

  signals:
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, QString player);

  private:
    //Note m_pTrackTablePlaceholder is defined in the .ui file
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    //QButtonGroup m_songsButtonGroup;
    WSelectorLibraryTableView* m_pSelectorLibraryTableView;
    SelectorLibraryTableModel* m_pSelectorLibraryTableModel;
    WSelectorCratesTableView* m_pSelectorCratesTableView;
    CrateView* m_pCrateView;
    QSqlTableModel* m_pCratesTableModel;
};

#endif //DLGSELECTOR_H
