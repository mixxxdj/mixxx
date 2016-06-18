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
    virtual void bindPaneWidget(WBaseLibrary* libraryWidget,
                                KeyboardEventFilter* pKeyboard);
    void bindSearchBar(WSearchLineEdit* pSearchLine);

    void addFeature(LibraryFeature* feature);
    void addFeatures(const QList<LibraryFeature*>& features);

    WBaseLibrary* getPaneWidget();

    void setFocusedFeature(const QString& featureName);

    QString getFocusedFeature() {
        return m_focusedFeature;
    }

    void setFocus();

    void clearFocus();

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

  protected:

    WBaseLibrary* m_pPaneWidget;
    
    QList<LibraryFeature*> m_features;

  private:

    const static QString m_sTrackViewName;

    QString m_focusedFeature;

  private slots:

    // Used to handle focus change
    bool eventFilter(QObject*, QEvent* event);
};

#endif // LIBRARYVIEWMANAGER_H
