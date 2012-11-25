#ifndef CRATEFEATURE_H
#define CRATEFEATURE_H

#include <QModelIndex>
#include <QList>
#include <QPair>
#include <QAction>

#include "library/libraryfeature.h"
#include "library/cratetablemodel.h"

#include "treeitemmodel.h"
#include "configobject.h"

class TrackCollection;

class CrateFeature : public LibraryFeature {
    Q_OBJECT
  public:
    CrateFeature(QObject* parent, TrackCollection* pTrackCollection, ConfigObject<ConfigValue>* pConfig);
    virtual ~CrateFeature();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QList<QUrl> urls);
    bool dropAcceptChild(const QModelIndex& index, QList<QUrl> urls);
    bool dragMoveAccept(QUrl url);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

    void bindWidget(WLibrary* libraryWidget,
                    MixxxKeyboard* keyboard);

    TreeItemModel* getChildModel();

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);
    void onLazyChildExpandation(const QModelIndex& index);

    void slotCreateCrate();
    void slotDeleteCrate();
    void slotRenameCrate();
    void slotToggleCrateLock();
    void slotImportPlaylist();
    void slotExportPlaylist();
    void slotCrateTableChanged(int playlistId);
    void htmlLinkClicked(const QUrl & link);

  private:
    QString getRootViewHtml() const;
    QModelIndex constructChildModel(int selected_id);
    void clearChildModel();
    void buildCrateList();

    TrackCollection* m_pTrackCollection;
    CrateDAO& m_crateDao;
    QAction *m_pCreateCrateAction;
    QAction *m_pDeleteCrateAction;
    QAction *m_pRenameCrateAction;
    QAction *m_pLockCrateAction;
    QAction *m_pImportPlaylistAction;
    QAction *m_pExportPlaylistAction;
    QList<QPair<int, QString> > m_crateList;
    CrateTableModel m_crateTableModel;
    QModelIndex m_lastRightClickedIndex;
    TreeItemModel m_childModel;
    ConfigObject<ConfigValue>* m_pConfig;
};

#endif /* CRATEFEATURE_H */
