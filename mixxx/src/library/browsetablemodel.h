#ifndef BROWSETABLEMODEL_H
#define BROWSETABLEMODEL_H

#include <QStandardItemModel>
#include "trackmodel.h"
class QMimeData;

class BrowseTableModel : public QStandardItemModel, public TrackModel
{
    
    Q_OBJECT
	public:
		BrowseTableModel(QObject* parent);
		~BrowseTableModel();
		
		void setPath(QString absPath);
		//reimplemented from TrackModel class
		virtual TrackPointer getTrack(const QModelIndex& index) const;
		virtual QString getTrackLocation(const QModelIndex& index) const;
		virtual void search(const QString& searchText);
		virtual void removeTrack(const QModelIndex& index);
        virtual void removeTracks(const QModelIndexList& indices);
        virtual bool addTrack(const QModelIndex& index, QString location);
        virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
        virtual const QString currentSearch();
        virtual bool isColumnInternal(int);
        virtual void moveTrack(const QModelIndex&, const QModelIndex&);
        virtual QItemDelegate* delegateForColumn(const int);
        virtual bool isColumnHiddenByDefault(int column);
        virtual const QList<int>& searchColumns() const;
	private:
	   //This method is executed in a Thread
	   void populateModel(QString absPath);
	   QMutex m_populationMutex;
	   
	   // A list of columns that the implementation wants searched
       QList<int> m_searchColumns;
       // Add a column to be searched when searching occurs
       void addSearchColumn(int index);
       

};

#endif
