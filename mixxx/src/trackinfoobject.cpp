#include "qstring.h"
#include "qdom.h"
#include <qfileinfo.h>

#include "trackinfoobject.h"


/*
	Initialize a new track with the filename.
*/
TrackInfoObject::TrackInfoObject( const QString sPath, const QString sFile ) : m_sFilename(sFile), m_sFilepath(sPath)
{
    m_sArtist = "";
    m_sTitle = "";
    m_sType= "";
    m_sComment = "";
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
    m_sFilename = SelectNode( nodeHeader, "Filename").toElement().text();
    m_sFilepath = SelectNode( nodeHeader, "Filepath").toElement().text();
    m_sTitle = SelectNode( nodeHeader, "Title").toElement().text();
    m_sArtist = SelectNode( nodeHeader, "Artist").toElement().text();
    m_sType = SelectNode( nodeHeader, "Type").toElement().text();
    m_sComment = SelectNode( nodeHeader, "Comment").toElement().text();
    m_iDuration = SelectNode( nodeHeader, "Duration").toElement().text().toInt();
    m_sBitrate = SelectNode( nodeHeader, "Bitrate").toElement().text();
    m_iLength = SelectNode( nodeHeader, "Length").toElement().text().toInt();
    m_iTimesPlayed = SelectNode( nodeHeader, "TimesPlayed").toElement().text().toInt();

    // Check that the actual file exists:
    CheckFileExists();
}

QDomNode TrackInfoObject::SelectNode( const QDomNode &nodeHeader, const QString sNode )
{
    QDomNode node = nodeHeader.firstChild();

    while ( !node.isNull() )
    {
        if (node.nodeName() == sNode)
            return node;
        node = node.nextSibling();
    }
    return node;
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
    QFile fileTrack( m_sFilepath+"/"+m_sFilename );
    if (fileTrack.exists() )
        m_bExist = true;
    else
    {
        m_bExist = false;
        qDebug("The track %s was not found", (m_sFilepath+"/"+m_sFilename).latin1());
    }
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
    AddElement( doc, header, "Comment", m_sComment);
	AddElement( doc, header, "Duration", QString("%1").arg(m_iDuration) );
	AddElement( doc, header, "Bitrate", m_sBitrate );
	AddElement( doc, header, "Length", QString("%1").arg(m_iLength) );
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
    It assumes that the filename is written like: "artist - trackname.xxx"
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

    // Sort out obvíously wrong parsings:
    if ((m_sArtist.length() < 3) || (m_sTitle < 3))
    {
        m_sTitle = m_sFilename.section('.',0,-2);
        m_sArtist = ' ';
    }

	// Find the length:
	m_iLength = QFileInfo( m_sFilepath + '/' + m_sFilename ).size();

    // Add no comment
    m_sComment = QString("");
}

/*
    Return the duration as a string with minutes and seconds: H:MM:SS
*/
QString TrackInfoObject::Duration()
{
    if (m_iDuration <=0)
        return QString("?");
    else
    {
        int iHours = m_iDuration/3600;
        int iMinutes = (m_iDuration - 3600*iHours)/60;
        int iSeconds = m_iDuration%60;
        
        // Sort out obviously wrong results:
        if (iHours > 5)
            return QString("??");
        if (iHours >= 1)
            return QString().sprintf("%2d:%2d:%02d", iHours, iMinutes, iSeconds);
        else
            return QString().sprintf("%2d:%02d", iMinutes, iSeconds);
    }
}

QString TrackInfoObject::Location()
{
    return m_sFilepath + "/" + m_sFilename;
}

QString TrackInfoObject::getInfo()
{
    return QString("" + m_sArtist + "\n" +
                   "" + m_sTitle + "\n");
//                   "Type   : " + m_sType  + "\n" +
//                   "Bitrate: " + m_sBitrate );
}
