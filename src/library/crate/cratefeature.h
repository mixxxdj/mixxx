#ifndef MIXXX_CRATEFEATURE_H
#define MIXXX_CRATEFEATURE_H


#include <QModelIndex>
#include <QList>
#include <QPair>
#include <QAction>
#include <QVariant>
#include <QUrl>
#include <QIcon>
#include <QPoint>

#include "library/crate/cratetablemodel.h"

#include "library/libraryfeature.h"
#include "library/library.h"
#include "library/treeitemmodel.h"

#include "track/track.h"

#include "preferences/usersettings.h"

// forward declaration(s)
class TrackCollection;


class CrateFeature : public LibraryFeature {
    Q_OBJECT
  public:
    CrateFeature(Library* pLibrary,
                 TrackCollection* pTrackCollection,
                 UserSettingsPointer pConfig);
    virtual ~CrateFeature();

    QVariant title();
    QIcon getIcon();

    bool dropAcceptChild(const QModelIndex& index, QList<QUrl> urls,
                         QObject* pSource);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

    void bindWidget(WLibrary* libraryWidget,
                    KeyboardEventFilter* keyboard);

    TreeItemModel* getChildModel();

  signals:
    void analyzeTracks(QList<TrackId>);

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void activateCrate(CrateId crateId);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);

    void slotCreateCrate();
    void slotDeleteCrate();
    void slotRenameCrate();
    void slotDuplicateCrate();
    void slotAutoDjTrackSourceChanged();
    void slotToggleCrateLock();
    void slotImportPlaylist();
    void slotImportPlaylistFile(const QString &playlist_file);
    void slotCreateImportCrate();
    void slotExportPlaylist();
    // Copy all of the tracks in a crate to a new directory (like a thumbdrive).
    void slotExportTrackFiles();
    void slotAnalyzeCrate();
    void slotCrateTableChanged(CrateId crateId);
    void slotCrateContentChanged(CrateId crateId);
    void htmlLinkClicked(const QUrl& link);

  private slots:
    void slotTrackSelected(TrackPointer pTrack);
    void slotResetSelectedTrack();
    void slotUpdateCrateLabels();

  private:
    QString getRootViewHtml() const;
    QModelIndex constructChildModel(CrateId selected_id = CrateId());
    void updateChildModel(CrateId selected_id);
    void clearChildModel();
    QVector<CrateSummary> buildCrateList();
    CrateId crateIdFromIndex(QModelIndex index);
    // Get the QModelIndex of a crate based on its id.  Returns QModelIndex()
    // on failure.
    QModelIndex indexFromCrateId(CrateId crateId);

    bool readLastRightClickedCrate(Crate* pCrate);

    TrackCollection* m_pTrackCollection;
    QAction *m_pCreateCrateAction;
    QAction *m_pDeleteCrateAction;
    QAction *m_pRenameCrateAction;
    QAction *m_pLockCrateAction;
    QAction *m_pDuplicateCrateAction;
    QAction *m_pAutoDjTrackSource;
    QAction *m_pImportPlaylistAction;
    QAction *m_pCreateImportPlaylistAction;
    QAction *m_pExportPlaylistAction;
    QAction *m_pExportTrackFilesAction;
    QAction *m_pAnalyzeCrateAction;
    QList<QPair<CrateId, QString> > m_crateList;
    CrateTableModel m_crateTableModel;
    QModelIndex m_lastRightClickedIndex;
    TreeItemModel m_childModel;
    TrackPointer m_pSelectedTrack;
};


#endif // MIXXX_CRATEFEATURE_H
