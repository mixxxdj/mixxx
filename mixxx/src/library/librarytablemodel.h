#ifndef LIBRARYTABLEMODEL_H
#define LIBRARYTABLEMODEL_H

#include <QtSql>
#include <QtCore>

#include "library/basesqltablemodel.h"
#include "library/trackmodel.h"
#include "library/dao/trackdao.h"

class TrackCollection;

class LibraryTableModel : public BaseSqlTableModel, public virtual TrackModel
{
    Q_OBJECT
  public:
    LibraryTableModel(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~LibraryTableModel();

    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual const QString currentSearch();
    virtual bool isColumnInternal(int column);
    virtual void removeTrack(const QModelIndex& index);
    virtual bool addTrack(const QModelIndex& index, QString location);
    virtual void moveTrack(const QModelIndex& sourceIndex,
                           const QModelIndex& destIndex);
    virtual QVariant data(const QModelIndex& item, int role) const;


    QMimeData* mimeData(const QModelIndexList &indexes) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QItemDelegate* delegateForColumn(const int i);
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    TrackModel::CapabilitiesFlags getCapabilities() const;
    static const QString DEFAULT_LIBRARYFILTER;
    void setLibraryPrefix(QString sPrefix);

  private:
    TrackDAO& m_trackDao;
    QString m_sPrefix;

  private slots:
    void slotSearch(const QString& searchText);
  signals:
    void doSearch(const QString& searchText);

  protected:
    QString m_currentSearch;
};

#endif
