#pragma once

#include <QFileDialog>
#include <QIcon>
#include <QList>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariant>

#include "library/coverartcache.h"
#include "library/dao/trackdao.h"
#include "library/treeitemmodel.h"
#include "track/track_decl.h"
#ifdef __STEM__
#include "engine/engine.h"
#endif

class KeyboardEventFilter;
class Library;
class WLibrary;
class WLibrarySidebar;
class QAbstractItemModel;

// pure virtual (abstract) class to provide an interface for libraryfeatures
class LibraryFeature : public QObject {
  Q_OBJECT
  public:
    LibraryFeature(
            Library* pLibrary,
            UserSettingsPointer pConfig,
            const QString& iconName);
    ~LibraryFeature() override = default;

    virtual QVariant title() = 0;

    /// Returns the icon name.
    ///
    /// This is useful for QML skins that need to build a URL anyway and may use their own icon theme.
    QString iconName() const {
        return m_iconName;
    }

    /// Returns the icon.
    ///
    /// This is used by legacy QWidget skins that display a QIcon directly.
    QIcon icon() const {
        return m_icon;
    }

    virtual bool dropAccept(const QList<QUrl>& urls, QObject* pSource) {
        Q_UNUSED(urls);
        Q_UNUSED(pSource);
        return false;
    }
    virtual bool dropAcceptChild(const QModelIndex& index,
            const QList<QUrl>& urls,
            QObject* pSource) {
        Q_UNUSED(index);
        Q_UNUSED(urls);
        Q_UNUSED(pSource);
        return false;
    }
    virtual bool dragMoveAccept(const QUrl& url) {
        Q_UNUSED(url);
        return false;
    }
    virtual bool dragMoveAcceptChild(const QModelIndex& index, const QUrl& url) {
        Q_UNUSED(index);
        Q_UNUSED(url);
        return false;
    }

    virtual void clear() {
    }
    virtual void paste() {
    }
    virtual void pasteChild(const QModelIndex& index) {
        Q_UNUSED(index);
    }
    // Reimplement this to register custom views with the library widget.
    virtual void bindLibraryWidget(WLibrary* /* libraryWidget */,
                            KeyboardEventFilter* /* keyboard */) {}
    virtual void bindSidebarWidget(WLibrarySidebar* /* sidebar widget */) {}
    virtual TreeItemModel* sidebarModel() const = 0;

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
    /// Pretend that the user has clicked on a tree item belonging
    /// to this LibraryFeature by updating both the library view
    /// and the sidebar selection.
    void selectAndActivate(const QModelIndex& index = QModelIndex());
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
    virtual void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) {
        Q_UNUSED(globalPos);
        Q_UNUSED(index);
    }
    // called when F2 key is pressed in WLibrarySidebar
    virtual void renameItem(const QModelIndex& index) {
        Q_UNUSED(index);
    }
    // called when Del or Backspace key is pressed in WLibrarySidebar
    virtual void deleteItem(const QModelIndex& index) {
        Q_UNUSED(index);
    }
    // Only implement this, if using incremental or lazy childmodels, see BrowseFeature.
    // This method is executed whenever you **double** click child items
    virtual void onLazyChildExpandation(const QModelIndex& index) {
        Q_UNUSED(index);
    }
  signals:
    void showTrackModel(QAbstractItemModel* model, bool restoreState = true);
    void switchToView(const QString& view);
    void loadTrack(TrackPointer pTrack);
#ifdef __STEM__
    void loadTrackToPlayer(TrackPointer pTrack,
            const QString& group,
            mixxx::StemChannelSelection stemMask,
            bool play = false);
#else
    void loadTrackToPlayer(TrackPointer pTrack,
            const QString& group,
            bool play = false);
#endif
    /// saves the scroll, selection and current state of the library model
    void saveModelState();
    /// restores the scroll, selection and current state of the library model
    void restoreModelState();
    void restoreSearch(const QString&);
    void disableSearch();
    void pasteFromSidebar();
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

    QString m_iconName;
    QIcon m_icon;
};
