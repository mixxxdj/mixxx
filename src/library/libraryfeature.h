// libraryfeature.h
// Created 8/17/2009 by RJ Ryan (rryan@mit.edu)

#ifndef LIBRARYFEATURE_H
#define LIBRARYFEATURE_H

#include <QAbstractItemModel>
#include <QDesktopServices>
#include <QFileDialog>
#include <QIcon>
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QtDebug>

#include "library/coverartcache.h"
#include "library/dao/trackdao.h"
#include "library/treeitemmodel.h"
#include "track/track_decl.h"

class KeyboardEventFilter;
class Library;
class TrackModel;
class WLibrary;
class WLibrarySidebar;

// pure virtual (abstract) class to provide an interface for libraryfeatures
class LibraryFeature : public QObject {
  Q_OBJECT
  public:
    LibraryFeature(
            Library* pLibrary,
            UserSettingsPointer pConfig);
    ~LibraryFeature() override = default;

    virtual QVariant title() = 0;
    virtual QIcon getIcon() = 0;

    virtual bool dropAccept(QList<QUrl> urls, QObject* pSource) {
        Q_UNUSED(urls);
        Q_UNUSED(pSource);
        return false;
    }
    virtual bool dropAcceptChild(const QModelIndex& index,
                                 QList<QUrl> urls, QObject* pSource) {
        Q_UNUSED(index);
        Q_UNUSED(urls);
        Q_UNUSED(pSource);
        return false;
    }
    virtual bool dragMoveAccept(QUrl url) {
        Q_UNUSED(url);
        return false;
    }
    virtual bool dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
        Q_UNUSED(index);
        Q_UNUSED(url);
        return false;
    }

    // Reimplement this to register custom views with the library widget.
    virtual void bindLibraryWidget(WLibrary* /* libraryWidget */,
                            KeyboardEventFilter* /* keyboard */) {}
    virtual void bindSidebarWidget(WLibrarySidebar* /* sidebar widget */) {}
    virtual TreeItemModel* getChildModel() = 0;

    virtual bool hasTrackTable() {
        return false;
    }

  protected:
    QStringList getPlaylistFiles() const {
        return getPlaylistFiles(QFileDialog::ExistingFiles);
    }
    QString getPlaylistFile() const {
        const QStringList playListFiles = getPlaylistFiles();
        if (playListFiles.isEmpty()) {
            return QString(); // no file chosen
        } else {
            return playListFiles.first();
        }
    }

    Library* const m_pLibrary;

    const UserSettingsPointer m_pConfig;

  public slots:
    // called when you single click on the root item
    virtual void activate() = 0;
    // called when you single click on a child item, e.g., a concrete playlist or crate
    virtual void activateChild(const QModelIndex& index) {
        Q_UNUSED(index);
    }
    // called when you right click on the root item
    virtual void onRightClick(const QPoint& globalPos) {
        Q_UNUSED(globalPos);
    }
    // called when you right click on a child item, e.g., a concrete playlist or crate
    virtual void onRightClickChild(const QPoint& globalPos, QModelIndex index) {
        Q_UNUSED(globalPos);
        Q_UNUSED(index);
    }
    // Only implement this, if using incremental or lazy childmodels, see BrowseFeature.
    // This method is executed whenever you **double** click child items
    virtual void onLazyChildExpandation(const QModelIndex& index) {
        Q_UNUSED(index);
    }
  signals:
    void showTrackModel(QAbstractItemModel* model);
    // A feature can emit this signal to tell the View to save state. This is usually not
    // required and is currently only needed when activating crates.
    void saveViewState();
    void switchToView(const QString& view);
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, QString group, bool play = false);
    void restoreSearch(const QString&);
    void disableSearch();
    // emit this signal before you parse a large music collection, e.g., iTunes, Traktor.
    // The second arg indicates if the feature should be "selected" when loading starts
    void featureIsLoading(LibraryFeature*, bool selectFeature);
    // emit this signal if the foreign music collection has been imported/parsed.
    void featureLoadingFinished(LibraryFeature*s);
    // emit this signal to select pFeature
    void featureSelect(LibraryFeature* pFeature, const QModelIndex& index);
    // emit this signal to enable/disable the cover art widget
    void enableCoverArtDisplay(bool);
    void trackSelected(TrackPointer pTrack);

  protected:
    // TODO: Move common crate/playlist functions into
    // a separate base class
    static bool exportPlaylistItemsIntoFile(
            QString playlistFilePath,
            const QList<QString>& playlistItemLocations,
            bool useRelativePath);

  private:
    QStringList getPlaylistFiles(QFileDialog::FileMode mode) const;
};

#endif /* LIBRARYFEATURE_H */
