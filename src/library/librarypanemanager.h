#ifndef LIBRARYVIEWMANAGER_H
#define LIBRARYVIEWMANAGER_H

#include <QObject>

#include "library/libraryfeature.h"
#include "widget/wbuttonbar.h"
#include "widget/wlibrary.h"
#include "widget/wsearchlineedit.h"
#include "widget/wtracktableview.h"

class LibraryPaneManager : public QObject {
    Q_OBJECT

  public:

    const int RIGHT_PANE_COUNT = 2;

    LibraryPaneManager(QObject* parent = nullptr);
    
    ~LibraryPaneManager();
    
    bool initialize();

    // All features must be added before adding a pane
    void bindLibraryWidget(WLibrary* libraryWidget, KeyboardEventFilter *pKeyboard);
    
    void bindTrackTable(WTrackTableView* pTrackTable);
    
    void bindSearchBar(WSearchLineEdit* pSearchLine);

    void addFeature(LibraryFeature* feature);
    
signals:
    
    void focused();
    
    void switchToView(const QString&);
    
    //void addFeatures(Q)

  public slots:

    void slotShowTrackModel(QAbstractItemModel* model);
    void slotSwitchToView(const QString& view);
    //void slotLoadTrack(TrackPointer pTrack);
    //void slotLoadTrackToPlayer(TrackPointer pTrack, QString group, bool play);
    //void slotLoadLocationToPlayer(QString location, QString group);
    void slotRestoreSearch(const QString& text);
    void slotRefreshLibraryModels();
    //void slotCreatePlaylist();
    //void slotCreateCrate();
    //void slotRequestAddDir(QString directory);
    //void slotRequestRemoveDir(QString directory, Library::RemovalType removalType);
    //void slotRequestRelocateDir(QString previousDirectory, QString newDirectory);
    void onSkinLoadFinished();
    void slotSetTrackTableFont(const QFont& font);
    void slotSetTrackTableRowHeight(int rowHeight);
  

  private:

    const static QString m_sTrackViewName;
    
    WLibrary* m_pLibraryWidget;
    WTrackTableView* m_pTrackTable;
    WSearchLineEdit* m_pSearchLine;
    
    QList<LibraryFeature*> m_features;
    
    
    QFont m_trackTableFont;
    int m_iTrackTableRowHeight;

  private slots:

    // Used to handle focus change
    // TODO(jmigual): Still needs to be implemented
    bool eventFilter(QObject*, QEvent* event);
};

#endif // LIBRARYVIEWMANAGER_H
