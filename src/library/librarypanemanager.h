#ifndef LIBRARYVIEWMANAGER_H
#define LIBRARYVIEWMANAGER_H

#include <QObject>
#include <QList>

#include "widget/wlibrary.h"
#include "widget/wsearchlineedit.h"
#include "widget/wtracktableview.h"

class LibraryFeature;
class WButtonBar;
class WLibraryBreadCrumb;
class TreeItem;
class Library;

class LibraryPaneManager : public QObject {
    Q_OBJECT

  public:

    LibraryPaneManager(int paneId, Library* pLibrary, QObject* parent = nullptr);

    ~LibraryPaneManager();

    bool initialize();

    // All features must be added before adding a pane
    virtual void bindPaneWidget(WBaseLibrary* pLibraryWidget,
                                KeyboardEventFilter* pKeyboard);
    void bindSearchBar(WSearchLineEdit* pSearchBar);
    void setBreadCrumb(WLibraryBreadCrumb* pBreadCrumb);

    void addFeature(LibraryFeature* feature);
    void addFeatures(const QList<LibraryFeature*>& features);

    WBaseLibrary* getPaneWidget();
    
    void setFocusedFeature(LibraryFeature* pFeature);
    LibraryFeature* getFocusedFeature() const;

    void setFocus();

    void clearFocus();
    
    inline int getPaneId() { 
        return m_paneId;
    }

  signals:

    void focused();

    void search(const QString& text);
    void searchCleared();
    void searchStarting();

  public slots:

    void slotSwitchToView(const QString& view);
    void slotSwitchToViewFeature(LibraryFeature* pFeature);
    void slotRestoreSearch(const QString& text);
    void slotShowBreadCrumb(TreeItem* pTree);
    void slotPaneCollapsed();
    void slotPaneUncollapsed();

  protected:

    QPointer<WBaseLibrary> m_pPaneWidget;
    QList<LibraryFeature*> m_features;
    QHash<LibraryFeature*, int> m_featuresWidget;
    QPointer<WLibraryBreadCrumb> m_pBreadCrumb;
    QPointer<WSearchLineEdit> m_pSearchBar;

  private:

    const static QString m_sTrackViewName;
    
    LibraryFeature* m_pFocusedFeature;
    int m_paneId;
    Library* m_pLibrary;

  private slots:

    // Used to handle focus change
    bool eventFilter(QObject*, QEvent* event);
};

#endif // LIBRARYVIEWMANAGER_H
