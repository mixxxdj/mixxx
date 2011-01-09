#ifndef CRATEFEATURE_H
#define CRATEFEATURE_H

#include <QSqlTableModel>
#include <QModelIndex>
#include <QAction>

#include "library/libraryfeature.h"
#include "library/cratetablemodel.h"

#include "treeitemmodel.h"

class TrackCollection;

class CrateFeature : public LibraryFeature {
    Q_OBJECT
  public:
    CrateFeature(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~CrateFeature();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QUrl url);
    bool dropAcceptChild(const QModelIndex& index, QUrl url);
    bool dragMoveAccept(QUrl url);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

    void bindWidget(WLibrarySidebar* sidebarWidget,
                    WLibrary* libraryWidget,
                    MixxxKeyboard* keyboard);

    TreeItemModel* getChildModel();
  signals:
    void showPage(const QUrl& page);

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);

    void slotCreateCrate();
    void slotDeleteCrate();
    void slotRenameCrate();
    void slotImportPlaylist();

  private:
    void constructChildModel();
    void clearChildModel();
  
    TrackCollection* m_pTrackCollection;
    QAction *m_pCreateCrateAction;
    QAction *m_pDeleteCrateAction;
    QAction *m_pRenameCrateAction;
    QAction *m_pImportPlaylistAction;
    QSqlTableModel m_crateListTableModel;
    CrateTableModel m_crateTableModel;
    QModelIndex m_lastRightClickedIndex;
    TreeItemModel m_childModel;
};

#endif /* CRATEFEATURE_H */
