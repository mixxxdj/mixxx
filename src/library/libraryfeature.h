// libraryfeature.h
// Created 8/17/2009 by RJ Ryan (rryan@mit.edu)

#ifndef LIBRARYFEATURE_H
#define LIBRARYFEATURE_H

#include <QFileDialog>
#include <QString>
#include <QUrl>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/coverartcache.h"
#include "library/dao/trackdao.h"
#include "preferences/usersettings.h"
#include "track/track.h"
#include "treeitemmodel.h"

class TrackModel;
class WBaseLibrary;
class WLibrary;
class Library;

// pure virtual (abstract) class to provide an interface for libraryfeatures
class LibraryFeature : public QObject {
    Q_OBJECT
  public:

    // The parent does not necessary be the Library
    LibraryFeature(UserSettingsPointer pConfig,
                   Library* pLibrary, 
                   QObject* parent = nullptr);
    
    virtual ~LibraryFeature();

    virtual QVariant title() = 0;
    virtual QIcon getIcon() = 0;
    
    // Must be a unique name for each feature, it must be a unique name for each
    // different feature
    virtual QString getViewName() = 0;

    virtual bool dropAccept(QList<QUrl> /* urls */, 
                            QObject* /* pSource */) {
        return false;
    }
    virtual bool dropAcceptChild(const QModelIndex& /* index */,
                                 QList<QUrl> /* urls */, 
                                 QObject* /* pSource */) {
        return false;
    }
    virtual bool dragMoveAccept(QUrl /* url */) {
        return false;
    }
    virtual bool dragMoveAcceptChild(const QModelIndex& /* index */, 
                                     QUrl /* url */) {
        return false;
    }
    
    // Reimplement this to register custom views with the library widget
    // at the right pane.
    virtual QWidget* createPaneWidget(KeyboardEventFilter* /* keyboard */, 
                                      int /* paneId */) {
        return nullptr;
    }
    
    // Reimplement this to register custom views with the library widget,
    // at the sidebar expanded pane
    virtual QWidget* createSidebarWidget(KeyboardEventFilter* pKeyboard);
    
    virtual TreeItemModel* getChildModel() = 0;
    
    void setFeatureFocus(int focus);
    
    int getFeatureFocus() {
        return m_featureFocus;
    }

  protected:
    inline QStringList getPlaylistFiles() { return getPlaylistFiles(QFileDialog::ExistingFiles); }
    inline QString getPlaylistFile() { return getPlaylistFiles(QFileDialog::ExistingFile).first(); }
    UserSettingsPointer m_pConfig;
    Library* m_pLibrary;
    
    int m_featureFocus;

  public slots:
    // called when you single click on the root item
    virtual void activate() = 0;
    // called when you single click on a child item, e.g., a concrete playlist or crate
    virtual void activateChild(const QModelIndex&) {
    }
    // called when you right click on the root item
    virtual void onRightClick(const QPoint&) {
    }
    // called when you right click on a child item, e.g., a concrete playlist or crate
    virtual void onRightClickChild(const QPoint& /* globalPos */, 
                                   QModelIndex /* index */) {
    }
    // Only implement this, if using incremental or lazy childmodels, see BrowseFeature.
    // This method is executed whenever you **double** click child items
    virtual void onLazyChildExpandation(const QModelIndex&) {
    }
    
  signals:
    void showTrackModel(QAbstractItemModel* model);
    void switchToView(const QString&);
    
    void loadTrack(TrackPointer);
    void loadTrackToPlayer(TrackPointer pTrack, QString group, bool play = false);
    void restoreSearch(const QString&);
    // emit this signal before you parse a large music collection, e.g., iTunes, Traktor.
    // The second arg indicates if the feature should be "selected" when loading starts
    void featureIsLoading(LibraryFeature*, bool selectFeature);
    // emit this signal if the foreign music collection has been imported/parsed.
    void featureLoadingFinished(LibraryFeature*);
    // emit this signal to select pFeature
    void featureSelect(LibraryFeature* pFeature, const QModelIndex& index);
    // emit this signal to enable/disable the cover art widget
    void enableCoverArtDisplay(bool);
    void trackSelected(TrackPointer);

  private: 
    QStringList getPlaylistFiles(QFileDialog::FileMode mode);

};

#endif /* LIBRARYFEATURE_H */
