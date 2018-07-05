#ifndef LIBRARYTREEMODEL_H
#define LIBRARYTREEMODEL_H

#include <QHash>
#include <QPixmap>
#include <QSqlQuery>
#include <QStringList>

#include "library/treeitemmodel.h"
#include "preferences/usersettings.h"

class CoverInfo;
class LibraryFeature;
class TrackCollection;

const QString LIBRARYTREEMODEL_SORT = "LibraryTree_Sort"; // ConfigValue key for Library Tree Model sort

class TracksTreeModel : public TreeItemModel {
    Q_OBJECT
  public:
    TracksTreeModel(LibraryFeature* pFeature,
                          TrackCollection* pTrackCollection,
                          UserSettingsPointer pConfig,
                          QObject* parent = nullptr);

    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role);

  public slots:
    void reloadTree() override;
    virtual void tracksAdded(const QSet<TrackId> trackIds);
    virtual void tracksRemoved(const QSet<TrackId> trackIds);
    virtual void trackChanged(TrackId trackId);

  protected:
    virtual void createTracksTree();
    virtual QString getGroupingOptions();
    bool removeTracksRecursive(const QSet<TrackId>& trackIds, TreeItem* pTree);

    parented_ptr<TreeItem> m_pGrouping;
    parented_ptr<TreeItem> m_pShowAll;

    LibraryFeature* m_pFeature;
    TrackCollection* m_pTrackCollection;
    UserSettingsPointer m_pConfig;

  private:
    struct CoverIndex {
        CoverIndex() {
            iCoverHash = iCoverLoc = iTrackLoc  = iCoverSrc = iCoverType = -1;
        }

        CoverIndex(const QSqlRecord& record);

        int iCoverHash;
        int iCoverLoc;
        int iTrackLoc;
        int iCoverSrc;
        int iCoverType;
    };

  private slots:
    void coverFound(const QObject* requestor, int requestReference, const CoverInfo&,
                    QPixmap pixmap, bool fromCache);

  private:
    QVariant getQuery(TreeItem* pTree) const;
    void addCoverArt(const CoverIndex& index, const QSqlQuery& query, TreeItem* pTree);
    QString createQueryStr(bool singleId = false);

    void insertTrackToTree(const QSqlQuery& query);

    QStringList m_sortOrder;
    QStringList m_coverQuery;
};

#endif // LIBRARYTREEMODEL_H
