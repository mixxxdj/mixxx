#ifndef TRACKINFOOBJECT_H
#define TRACKINFOOBJECT_H

#include <qobject.h>
class QString;
class QDomElement;

class TrackInfoObject : public QObject
{
	Q_OBJECT
public:
	TrackInfoObject( const QString );
	~TrackInfoObject();
	void ReadFromXML( const QDomElement & );
	void WriteToXML( const QDomElement & );
	void Parse();

	bool bExist; // Flag which determines if the file exist or not.
	QString sFilename;
	QString sArtist;
	QString sTitle;
	QString sType;
	QString sDuration;
    int iDurationSeconds;
};

#endif