#include "qstring.h"
#include "qdom.h"
#include <qfile.h>

#include "trackinfoobject.h"

/*
	Initialize a new track with the filename.
*/
TrackInfoObject::TrackInfoObject( const QString sPath, const QString sFile ) :
m_sFilepath(sPath), m_sFilename(sFile)
{
	m_sArtist = "";
	m_sTitle = "";
	m_sType= "";
	m_iDuration = 0;
	m_iLength = 0;
	m_sBitrate = "";
	m_iTimesPlayed = 0;

	// Check that the file exists:
	CheckFileExists();
}

/*
	Creates a new track given information from the xml file:
*/
TrackInfoObject::TrackInfoObject( const QDomNode &nodeHeader )
{
	// Read information:
	QDomNode node = nodeHeader.firstChild();

	m_sFilename = node.toElement().text();
	node = node.nextSibling();

	m_sFilepath = node.toElement().text();
	node = node.nextSibling();

	m_sTitle = node.toElement().text();
	node = node.nextSibling();

	m_sArtist = node.toElement().text();
	node = node.nextSibling();

	m_sType = node.toElement().text();
	node = node.nextSibling();
	
	m_iDuration = node.toElement().text().toInt();
	node = node.nextSibling();

	m_sBitrate = node.toElement().text();
	node = node.nextSibling();

	m_iTimesPlayed = node.toElement().text().toInt();

	// Check that the actual file exists:
	CheckFileExists();
}

TrackInfoObject::~TrackInfoObject()
{

}

/*
	Checks if the file given in m_sFilename really exists on the disc, and
	updates the m_bExists flag accordingly.
*/
void TrackInfoObject::CheckFileExists()
{
	QFile fileTrack( m_sFilename );
	if (fileTrack.exists() )
		m_bExist = true;
	else
		m_bExist = false;
}

/*
	Writes information about the track to the xml file:
*/
void TrackInfoObject::WriteToXML( QDomDocument &doc, QDomElement &header )
{
	AddElement( doc, header, "Filename", m_sFilename );
	AddElement( doc, header, "Filepath", m_sFilepath );
	AddElement( doc, header, "Title", m_sTitle );
	AddElement( doc, header, "Artist", m_sArtist );
	AddElement( doc, header, "Type", m_sType );
	AddElement( doc, header, "Duration", QString("%1").arg(Duration()) );
	AddElement( doc, header, "Bitrate", m_sBitrate );
	AddElement( doc, header, "TimesPlayed", QString("%1").arg(m_iTimesPlayed) );
}

/*
	Adds another element to the xml file. Only used by WriteToXML.
*/
void TrackInfoObject::AddElement( QDomDocument &doc, QDomElement &header,
								  QString sElementName, QString sText )
{
    QDomElement element = doc.createElement( sElementName );
	element.appendChild( doc.createTextNode( sText ) );
    header.appendChild( element );
}

/*
	Dummy method for parsing information from knowing only the file name.
*/
void TrackInfoObject::Parse()
{
	if (m_sFilename.find('-') != -1)
	{
		m_sArtist = m_sFilename.section('-',0,0); // Get the first part
		m_sTitle = m_sFilename.section('-',1,1); // Get the second part
		m_sTitle = m_sTitle.section('.',0,-2); // Remove the ending
		m_sType = m_sFilename.section('.',-1); // Get the ending
	} 
	else 
	{
		m_sTitle = m_sFilename.section('.',0,-2); // Remove the ending;
		m_sType = m_sFilename.section('.',-1); // Get the ending
	}

	// Find the length:
	m_iLength = QFile( m_sFilepath + '/' + m_sFilename ).size();
}

QString TrackInfoObject::Duration()
{
	return QString("%1:%2").arg( (int) (m_iDuration/60), 2 ).arg( m_iDuration%60, 2);
}
