#ifndef TRACKINFOOBJECT_H
#define TRACKINFOOBJECT_H

#include <qobject.h>
class QString;
class QDomElement;
class QDomDocument;
class QDomNode;

class TrackInfoObject : public QObject
{
	Q_OBJECT
public:
	TrackInfoObject( const QString );
	TrackInfoObject( const QDomNode & );
	~TrackInfoObject();
	void WriteToXML( QDomDocument &, QDomElement & );
	void Parse();

	bool m_bExist; // Flag which determines if the file exist or not.
	QString m_sFilename;
	QString m_sFilepath;
	QString m_sArtist;
	QString m_sTitle;
	QString m_sType;
	QString m_sDuration;
	QString m_sBitrate;
	int m_iTimesPlayed;

private:
	void AddElement( QDomDocument &, QDomElement &, QString, QString );
	void CheckFileExists();

};

#endif