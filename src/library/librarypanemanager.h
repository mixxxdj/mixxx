#ifndef LIBRARYVIEWMANAGER_H
#define LIBRARYVIEWMANAGER_H

#include <QObject>
#include <QList>

#include "library/libraryfeature.h"
#include "widget/wbuttonbar.h"
#include "widget/wlibrary.h"
#include "widget/wsearchlineedit.h"
#include "widget/wtracktableview.h"

class LibraryPaneManager : public QObject {
    Q_OBJECT

  public:
    
    enum class FeaturePane {
        SidebarExpanded,
        TrackTable
    };

    LibraryPaneManager(QObject* parent = nullptr);
    
    ~LibraryPaneManager();
    
    bool initialize();

    // All features must be added before adding a pane
    void bindLibraryWidget(WLibrary* libraryWidget, 
                           KeyboardEventFilter *pKeyboard, FeaturePane pane);
    void bindTrackTable(WTrackTableView* pTrackTable);
    void bindSearchBar(WSearchLineEdit* pSearchLine);
    
    void addFeature(LibraryFeature* feature);
    void addFeatures(const QList<LibraryFeature *> &features);
    
signals:
    
    void focused();
    
    void showTrackModel(QAbstractItemModel* model);
    void switchToView(const QString&);
    
    void restoreSearch(const QString&);
    void search(const QString& text);
    void searchCleared();
    void searchStarting();

  public slots:

    void slotShowTrackModel(QAbstractItemModel* model);
    void slotSwitchToView(const QString& view);
    void slotRestoreSearch(const QString& text);  

  private:

    const static QString m_sTrackViewName;
    
    WLibrary* m_pLibraryWidget;
    WTrackTableView* m_pTrackTable;
    WSearchLineEdit* m_pSearchLine;
    
    QList<LibraryFeature*> m_features;

  private slots:

    // Used to handle focus change
    // TODO(jmigual): Still needs to be implemented
    bool eventFilter(QObject*, QEvent* event);
};

#endif // LIBRARYVIEWMANAGER_H
