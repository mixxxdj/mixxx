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

class LibraryPaneManager : public QObject {
    Q_OBJECT

  public:

    LibraryPaneManager(int paneId, QObject* parent = nullptr);

    ~LibraryPaneManager();

    bool initialize();

    // All features must be added before adding a pane
    virtual void bindPaneWidget(WBaseLibrary* pLibraryWidget,
                                KeyboardEventFilter* pKeyboard);
    void bindSearchBar(WSearchLineEdit* pSearchLine);
    void setBreadCrumb(WLibraryBreadCrumb* pBreadCrumb);

    void addFeature(LibraryFeature* feature);
    void addFeatures(const QList<LibraryFeature*>& features);

    WBaseLibrary* getPaneWidget();

    void setFocusedFeature(const QString& featureName);

    QString getFocusedFeature() {
        return m_focusedFeature;
    }

    void setFocus();

    void clearFocus();
    
    inline int getPaneId() { 
        return m_paneId;
    }

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
    void slotSwitchToViewFeature(LibraryFeature* pFeature);
    void slotRestoreSearch(const QString& text);
    void slotShowBreadCrumb(TreeItem* pTree);

  protected:

    WBaseLibrary* m_pPaneWidget;
    QList<LibraryFeature*> m_features;
    QHash<LibraryFeature*, int> m_featuresWidget;
    WLibraryBreadCrumb* m_pBreadCrumb;

  private:

    const static QString m_sTrackViewName;

    QString m_focusedFeature;
    
    int m_paneId;

  private slots:

    // Used to handle focus change
    bool eventFilter(QObject*, QEvent* event);
};

#endif // LIBRARYVIEWMANAGER_H
