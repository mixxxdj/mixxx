// rekordboxfeature.h
// Created 05/24/2019 by Evan Dekker

// This feature reads tracks, playlists and folders from removable Recordbox 
// prepared devices (USB drives, etc), by parsing the binary *.PDB files
// stored on each removable device. It does not read the locally stored
// Rekordbox database (Collection).

// It draws heavily from the hard work completed here:

//      https://github.com/Deep-Symmetry/crate-digger

// And uses the C++ Kaitai Struct binary parsing libraries:

//      http://kaitai.io
//      https://github.com/kaitai-io/kaitai_struct
//      https://github.com/kaitai-io/kaitai_struct_cpp_stl_runtime

// The *.PDB C++ files:

//      rekordbox_pdb.h
//      rekordbox_pdb.cpp

// Were generated from the following structure definition file:

//      https://github.com/Deep-Symmetry/crate-digger/blob/master/src/main/kaitai/rekordbox_pdb.ksy

#ifndef REKORDBOX_FEATURE_H
#define REKORDBOX_FEATURE_H

#include <QStringListModel>
#include <QtSql>
#include <QXmlStreamReader>
#include <QFuture>
#include <QtConcurrentRun>
#include <QFutureWatcher>

#include <fstream>

#include "library/baseexternallibraryfeature.h"
#include "library/baseexternaltrackmodel.h"
#include "library/baseexternalplaylistmodel.h"
#include "library/treeitemmodel.h"

#include "library/rekordbox/rekordbox_pdb.h"

#define PDB_PATH "PIONEER/rekordbox/export.pdb"

class TrackCollection;
class BaseExternalPlaylistModel;

class RekordboxPlaylistModel : public BaseExternalPlaylistModel {
  public:
    RekordboxPlaylistModel(QObject* parent,
                         TrackCollection* pTrackCollection,
                         QSharedPointer<BaseTrackCache> trackSource);
    TrackPointer getTrack(const QModelIndex& index) const override;
    virtual bool isColumnHiddenByDefault(int column);
};

class RekordboxFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    RekordboxFeature(QObject* parent, TrackCollection*);
    virtual ~RekordboxFeature();

    QVariant title();
    QIcon getIcon();
    static bool isSupported();
    void bindWidget(WLibrary* libraryWidget,
                    KeyboardEventFilter* keyboard) override;

    TreeItemModel* getChildModel();

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void refreshLibraryModels();
    void onRekordboxDevicesFound();
    void onTracksFound();

  private slots:
      void htmlLinkClicked(const QUrl& link);    

  private:
    QString formatRootViewHtml() const;   
    virtual BaseSqlTableModel* getPlaylistModelForPlaylist(QString playlist);    
    QList<TreeItem*> findRekordboxDevices();
    QString parseDeviceDB(TreeItem *deviceItem);
    void buildPlaylistTree(TreeItem *parent, uint32_t parentID, 
        std::map<uint32_t, std::string> &playlistNameMap, 
        std::map<uint32_t, bool> &playlistIsFolderMap, 
        std::map<uint32_t, std::map<uint32_t, uint32_t>> &playlistTreeMap,
        std::map<uint32_t, std::map<uint32_t, uint32_t>> &playlistTrackMap,
        QString playlistPath, QString device);
    void clearTable(QString table_name);
    template<typename Base, typename T>
    inline bool instanceof(const T *ptr) {return dynamic_cast<const Base*>(ptr) != nullptr;}
    std::string getText(rekordbox_pdb_t::device_sql_string_t *deviceString);

    TreeItemModel m_childModel;
    TrackCollection* m_pTrackCollection;
    // A separate db connection for the worker parsing thread
    QSqlDatabase m_database;
    RekordboxPlaylistModel* m_pRekordboxPlaylistModel;

    QFutureWatcher<QList<TreeItem*>> m_devicesFutureWatcher;
    QFuture<QList<TreeItem*>> m_devicesFuture;
    QFutureWatcher<QString> m_tracksFutureWatcher;
    QFuture<QString> m_tracksFuture;
    QString m_title;

    QSharedPointer<BaseTrackCache> m_trackSource;
    QIcon m_icon;
};

#endif // REKORDBOX_FEATURE_H
