#ifndef LIBRARYVIEWMANAGER_H
#define LIBRARYVIEWMANAGER_H

#include <QObject>
#include <QList>

#include "../widget/wlibrarypane.h"
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

    void setFocused(bool value);
    
    void restoreSearch(const QString& text);
    void restoreSaveButton();
    void switchToFeature(LibraryFeature* pFeature);
    void showBreadCrumb(TreeItem* pTree);
    void showBreadCrumb(const QString& text, const QIcon &icon);
    
    int getPaneId() const;
    
    void setPreselected(bool value);
    bool isPreselected() const;
    
    void setPreviewed(bool value);

    bool focusSearch();

  signals:
    
    void searchCleared();
    void searchStarting();

  public slots:

    void slotPanePreselected(bool value);
    void slotPaneCollapsed();
    void slotPaneUncollapsed();
    void slotPaneFocused();
    void slotSearch(const QString& text);
    void slotSearchStarting();
    void slotSearchCleared();
    void slotSearchCancel();

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
