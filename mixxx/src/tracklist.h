#ifndef TRACKLIST_H
#define TRACKLIST_H

#include <qobject.h>
#include <qdom.h>

class QString;
//class QDomDocument;
class TrackInfoObject;
class QPtrList;

class TrackList : public QObject
{
	Q_OBJECT
public:
	TrackList( const QString );
	~TrackList();
	void WriteXML();

private:
	TrackInfoObject *FileExistsInList( const QString );
	void ReadXML ();
	void AddFiles(const char *);

	QString sDirectory;
	QDomDocument domXML;
	QPtrList<TrackInfoObject> lTracks; 
};

#endif