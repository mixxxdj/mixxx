#include "qstring.h"
#include "qdom.h"
#include <qfile.h>

#include "trackinfoobject.h"

/*
	Initialize a new track with the filename.
*/
TrackInfoObject::TrackInfoObject( const QString sFile )
{
	m_sFilename = sFile;
	m_sArtist = "";
	m_sTitle = "";
	m_sType= "";
	m_sDuration = "??:??";
	m_iDurationSeconds = 0;

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

	m_sTitle = node.toElement().text();
	node = node.nextSibling();

	m_sArtist = node.toElement().text();
	node = node.nextSibling();

	m_sType = node.toElement().text();
	node = node.nextSibling();
	
	m_sDuration = node.toElement().text();
	node = node.nextSibling();

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
	AddElement( doc, header, "Title", m_sTitle );
	AddElement( doc, header, "Artist", m_sArtist );
	AddElement( doc, header, "Type", m_sType );
	AddElement( doc, header, "Duration", m_sDuration );
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
	Dummy method for parsing information from the file.
*/
void TrackInfoObject::Parse()
{
	m_sTitle = m_sFilename;
}
