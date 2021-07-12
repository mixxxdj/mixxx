#pragma once

#include <QAction>
#include <QModelIndex>
#include <QPointer>

#include "library/libraryfeature.h"
#include "library/dao/playlistdao.h"
#include "util/parented_ptr.h"

class BaseSqlTableModel;
class TrackCollection;

class BaseExternalLibraryFeature : public LibraryFeature {
    Q_OBJECT
  public:
    BaseExternalLibraryFeature(
            Library* pLibrary,
            UserSettingsPointer pConfig,
            const QString& iconName);
    ~BaseExternalLibraryFeature() override = default;

  public slots:
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;
    void onRightClick(const QPoint& globalPos) override;
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) override;

  protected:
    // Must be implemented by external Libraries copied to Mixxx DB
    virtual BaseSqlTableModel* getPlaylistModelForPlaylist(const QString& playlist) {
        Q_UNUSED(playlist);
        return nullptr;
    }
    // Must be implemented by external Libraries not copied to Mixxx DB
    virtual void appendTrackIdsFromRightClickIndex(QList<TrackId>* trackIds, QString* pPlaylist);

  private slots:
    void slotAddToAutoDJ();
    void slotAddToAutoDJTop();
    void slotAddToAutoDJReplace();
    void slotImportAsMixxxPlaylist();

  protected:
    QModelIndex lastRightClickedIndex() const {
        return m_lastRightClickedIndex;
    }

    TrackCollection* const m_pTrackCollection;

  private:
    void addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc);

    QModelIndex m_lastRightClickedIndex;

    parented_ptr<QAction> m_pAddToAutoDJAction;
    parented_ptr<QAction> m_pAddToAutoDJTopAction;
    parented_ptr<QAction> m_pAddToAutoDJReplaceAction;
    parented_ptr<QAction> m_pImportAsMixxxPlaylistAction;

    QPointer<WLibrarySidebar> m_pSidebarWidget;
};
