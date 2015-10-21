#ifndef CRATEFEATURE_H
#define CRATEFEATURE_H

#include <QModelIndex>
#include <QList>
#include <QPair>
#include <QAction>
#include <QVariant>
#include <QUrl>
#include <QIcon>
#include <QPoint>
#include <QSet>

#include "library/libraryfeature.h"
#include "library/cratetablemodel.h"
#include "library/library.h"

#include "treeitemmodel.h"
#include "configobject.h"
#include "trackinfoobject.h"

class TrackCollection;

class CrateFeature : public LibraryFeature {
    Q_OBJECT
  public:
    CrateFeature(Library* pLibrary,
                 TrackCollection* pTrackCollection,
                 ConfigObject<ConfigValue>* pConfig);
    virtual ~CrateFeature();

    QVariant title();
    QIcon getIcon();

    bool dropAcceptChild(const QModelIndex& index, QList<QUrl> urls,
                         QObject* pSource);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

    void bindWidget(WLibrary* libraryWidget,
                    MixxxKeyboard* keyboard);

    TreeItemModel* getChildModel();

  signals:
    void analyzeTracks(QList<TrackId>);

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void activateCrate(int crateId);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);

    void slotCreateCrate();
    void slotDeleteCrate();
    void slotRenameCrate();
    void slotDuplicateCrate();
    void slotAutoDjTrackSourceChanged();
    void slotToggleCrateLock();
    void slotImportPlaylist();
    void slotExportPlaylist();
    void slotAnalyzeCrate();
    void slotCrateTableChanged(int crateId);
    void slotCrateContentChanged(int crateId);
    void slotCrateTableRenamed(int crateId, QString a_strName);
    void htmlLinkClicked(const QUrl& link);

  private slots:
    void slotTrackSelected(TrackPointer pTrack);
    void slotResetSelectedTrack();

  private:
    QString getRootViewHtml() const;
    QModelIndex constructChildModel(int selected_id);
    void updateChildModel(int selected_id);
    void clearChildModel();
    void buildCrateList();
    int crateIdFromIndex(QModelIndex index);
    // Get the QModelIndex of a crate based on its id.  Returns QModelIndex()
    // on failure.
    QModelIndex indexFromCrateId(int crateId);

    TrackCollection* m_pTrackCollection;
    CrateDAO& m_crateDao;
    QAction *m_pCreateCrateAction;
    QAction *m_pDeleteCrateAction;
    QAction *m_pRenameCrateAction;
    QAction *m_pLockCrateAction;
    QAction *m_pDuplicateCrateAction;
#ifdef __AUTODJCRATES__
    QAction *m_pAutoDjTrackSource;
#endif // __AUTODJCRATES__
    QAction *m_pImportPlaylistAction;
    QAction *m_pExportPlaylistAction;
    QAction *m_pAnalyzeCrateAction;
    QList<QPair<int, QString> > m_crateList;
    CrateTableModel m_crateTableModel;
    QModelIndex m_lastRightClickedIndex;
    TreeItemModel m_childModel;
    ConfigObject<ConfigValue>* m_pConfig;
    TrackPointer m_pSelectedTrack;
    QSet<int> m_cratesSelectedTrackIsIn;
};

#endif /* CRATEFEATURE_H */
