#ifndef TRACKLIST_H
#define TRACKLIST_H

#include <qobject.h>

class QString;
class TrackInfoObject;
class QPtrList;

class TrackList : public QObject
{
	Q_OBJECT
public:
	TrackList( const QString );
	~TrackList();
	void WriteXML( );

private:
	TrackInfoObject *FileExistsInList( const QString );
	void ReadXML ();
	bool AddFiles(const char *);

	QString m_sDirectory; // the directory where the music files are stored
	QPtrList<TrackInfoObject> m_lTracks; // list of all tracks
};

#endif