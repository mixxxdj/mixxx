#include "qstring.h"
#include "qdom.h"
#include <qfile.h>

#include "trackinfoobject.h"

/*
	Initialize a new track with the filename.
*/
TrackInfoObject::TrackInfoObject( const QString sFile )
{
	sFilename = sFile;
	sArtist = "";
	sTitle = "";
	sType= "";
	sDuration = "??:??";
	iDurationSeconds = 0;

	// Check that the file exists:
	QFile fileTrack( sFilename );
	if (fileTrack.exists() )
		bExist = true;
	else
		bExist = false;
}

TrackInfoObject::~TrackInfoObject()
{

}

/*
	Reads all the available information from the given node.
*/
void TrackInfoObject::ReadFromXML( const QDomElement &header )
{
	QDomNode node = header.firstChild();
	while ( !node.isNull() ) {
		if ( node.isElement() ) {
			// case for the different entries
            if ( node.nodeName() == "title" ) 
                sTitle = node.firstChild().nodeValue();
            if ( node.nodeName() == "artist" )
				sArtist = node.firstChild().nodeValue();
        }
        node = node.nextSibling();
    }
} 

void TrackInfoObject::WriteToXML( const QDomElement &header )
{
}

/*
	Dummy method for parsing information from the file.
*/
void TrackInfoObject::Parse()
{
	sTitle = sTitle;
}
