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
    
    void setCurrentFeature(LibraryFeature* pFeature);
    LibraryFeature* getCurrentFeature() const;

    void setFocus();
    void clearFocus();
    
    void restoreSearch(const QString& text);
    void switchToFeature(LibraryFeature* pFeature);
    void showBreadCrumb(TreeItem* pTree);
    
    inline int getPaneId() { 
        return m_paneId;
    }

  signals:

    void search(const QString& text);
    void searchCleared();
    void searchStarting();

  public slots:

    void slotPaneCollapsed();
    void slotPaneUncollapsed();
    void slotPaneFocused();

  protected:

    QPointer<WBaseLibrary> m_pPaneWidget;
    QList<LibraryFeature*> m_features;
    QPointer<WLibraryBreadCrumb> m_pBreadCrumb;
    QPointer<WSearchLineEdit> m_pSearchBar;

  private:
    
    LibraryFeature* m_pCurrentFeature;
    int m_paneId;
    Library* m_pLibrary;

  private slots:

    // Used to handle focus change
    bool eventFilter(QObject*, QEvent* event);
};

#endif // LIBRARYVIEWMANAGER_H
