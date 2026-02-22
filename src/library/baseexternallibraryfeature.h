#pragma once

#include <QAction>
#include <QModelIndex>
#include <QPointer>
#include <memory>

#include "library/dao/playlistdao.h"
#include "library/libraryfeature.h"
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
    // Must be re-implemented by external Libraries copied to Mixxx DB
    virtual std::unique_ptr<BaseSqlTableModel> createPlaylistModelForPlaylist(
            const QVariant& data);
    // Must be implemented by external Libraries not copied to Mixxx DB
    virtual void appendTrackIdsFromRightClickIndex(QList<TrackId>* trackIds,
            QString* pPlaylist);

  private slots:
    void slotAddToAutoDJ();
    void slotAddToAutoDJTop();
    void slotAddToAutoDJReplace();
    void slotImportAsMixxxPlaylist();
    void slotImportAsMixxxCrate();

  protected:
    QModelIndex lastRightClickedIndex() const {
        return m_lastRightClickedIndex;
    }
    void clearLastRightClickedIndex() {
        m_lastRightClickedIndex = QModelIndex();
    };

    TrackCollection* const m_pTrackCollection;

  private:
    void addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc);

    // Caution: Make sure this is reset whenever the library tree is updated,
    // so that the internalPointer() does not become dangling
    QModelIndex m_lastRightClickedIndex;

    parented_ptr<QAction> m_pAddToAutoDJAction;
    parented_ptr<QAction> m_pAddToAutoDJTopAction;
    parented_ptr<QAction> m_pAddToAutoDJReplaceAction;
    parented_ptr<QAction> m_pImportAsMixxxPlaylistAction;
    parented_ptr<QAction> m_pImportAsMixxxCrateAction;

    QPointer<WLibrarySidebar> m_pSidebarWidget;
};
