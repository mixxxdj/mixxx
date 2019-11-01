#ifndef BASEEXTERNALLIBRARYFEATURE_H
#define BASEEXTERNALLIBRARYFEATURE_H

#include <QAction>
#include <QModelIndex>
#include <QPointer>

#include "library/libraryfeature.h"

class BaseSqlTableModel;
class TrackCollection;

class BaseExternalLibraryFeature : public LibraryFeature {
    Q_OBJECT
  public:
    BaseExternalLibraryFeature(
            Library* pLibrary,
            UserSettingsPointer pConfig);
    ~BaseExternalLibraryFeature() override;

  public slots:
    virtual void bindSidebarWidget(WLibrarySidebar* pSidebarWidget);
    virtual void onRightClick(const QPoint& globalPos);
    virtual void onRightClickChild(const QPoint& globalPos, QModelIndex index);

  protected:
    // Must be implemented by external Libraries copied to Mixxx DB
    virtual BaseSqlTableModel* getPlaylistModelForPlaylist(QString playlist) {
        Q_UNUSED(playlist);
        return NULL;
    }
    // Must be implemented by external Libraries not copied to Mixxx DB
    virtual void appendTrackIdsFromRightClickIndex(QList<TrackId>* trackIds, QString* pPlaylist);

    TrackCollection* const m_pTrackCollection;

    QModelIndex m_lastRightClickedIndex;

  private slots:
    void slotAddToAutoDJ();
    void slotAddToAutoDJTop();
    void slotImportAsMixxxPlaylist();

  private:
    void addToAutoDJ(bool bTop);

    QAction* m_pAddToAutoDJAction;
    QAction* m_pAddToAutoDJTopAction;
    QAction* m_pImportAsMixxxPlaylistAction;

    QPointer<WLibrarySidebar> m_pSidebarWidget;
};

#endif // BASEEXTERNALLIBRARYFEATURE_H
