/***************************************************************************
                          trackinfoobject.h  -  description
                             -------------------
    begin                : 10 02 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TRACKINFOOBJECT_H
#define TRACKINFOOBJECT_H

#include <qobject.h>

class QString;
class QDomElement;
class QDomDocument;
class QDomNode;
class WTrackTable;
class WTrackTableItem;


class TrackInfoObject : public QObject
{
    Q_OBJECT
public:
    /** Initialize a new track with the filename. */
    TrackInfoObject(const QString sPath, const QString sFile);
    /** Creates a new track given information from the xml file. */
    TrackInfoObject(const QDomNode &);
    ~TrackInfoObject();
    int parse();
    /** Checks if the file given in m_sFilename really exists on the disc, and
        updates the m_bExists flag accordingly. */
    void checkFileExists();
    void writeToXML( QDomDocument &, QDomElement & );
    /** Utility function to get a node from an xml file: */
    static QDomNode selectNode( const QDomNode &, const QString );
    /** Utility function to get a the text of an element in a node from an xml file: */
    static QString selectNodeStr( const QDomNode &, const QString );

    /** Insert at the values in a WTrackTable at a given row */
    void insertInTrackTableRow(WTrackTable *pTableTrack, int iRow);
    /** Reset pointers to table cells */
    void removeFromTrackTable();
    /** Returns the duration in seconds */
    int getDuration();
    /** Returns the duration as a string: H:MM:SS */
    QString getDurationStr();
    /** Returns the location of the file, included path */
    QString getLocation();
    /** Returns BPM */
    float getBpm();
    /** Set BPM */
    void setBpm(float);
    /** Returns BPM as a string */
    QString getBpmStr();
    /** Retruns BPM confidence */
    float getBpmConfidence();
    /** Set BPM confidence */
    void setBpmConfidence(float f);
    /** Returns the user comment */
    QString getComment();
    /** Sets the user commnet */
    void setComment(QString);
    /** Returns the file type */
    QString getType();
    /** Sets the type of the string */
    void setType(QString);
    /** Returns the bitrate */
    int getBitrate();
    /** Returns the bitrate as a string */
    QString getBitrateStr();
    /** Sets the bitrate */
    void setBitrate(int);
    /** Retruns the length of the file in bytes */
    int getLength();
    /** Output a formatted string with all the info */
    QString getInfo();
    /** Set duration in seconds */
    void setDuration(int);
    /** Return title */
    QString getTitle();
    /** Set title */
    void setTitle(QString);
    /** Return artist */
    QString getArtist();
    /** Set artist */
    void setArtist(QString);
    /** Return filename */
    QString getFilename();
    /** Return true if the file exist */
    bool exists();
    /** Return number of times the track has been played */
    int getTimesPlayed();
    /** Increment times played with one */
    void incTimesPlayed();
    /** Sets the filepath */
    void setFilepath(QString);
    /** Returns the score as string */
    QString getScoreStr();
    /** Sets the score */
    void setScore(int);
    /** Get id */
    int getId();
    /** Set id */
    void setId(int iId);
    /** Return the next track as listed in WTrackTable */
    TrackInfoObject *getNext();


private:
    /** Method for parsing information from knowing only the file name.
        It assumes that the filename is written like: "artist - trackname.xxx" */
    void parseFilename();
    /** Flag which determines if the file exists or not. */
    bool m_bExists;
    /** File name */
    QString m_sFilename;
    /** Path for the track file */
    QString m_sFilepath;
    /** Artist */
    QString m_sArtist;
    /** Title */
    QString m_sTitle;
    /** File type */
    QString m_sType;
    /** User comment */
    QString m_sComment;
    /** Duration of track in seconds */
    int m_iDuration;
    /** Length of track in bytes */
    int m_iLength;
    /** Bitrate */
    int m_iBitrate;
    /** Number of times the track has been played */
    int m_iTimesPlayed;
    /** Beat per minutes (BPM) */
    float m_fBpm;
    /** Confidence measure of BPM */
    float m_fBpmConfidence;
    /** Score. Reflects the relative number of times the track has been played */
    float m_iScore;
    /** Id. Unique ID of track */
    int m_iId;

    /** WTrackTableItems are representations of the values actually shown in the WTrackTable */
    WTrackTableItem *m_pTableItemScore, *m_pTableItemTitle, *m_pTableItemArtist, *m_pTableItemComment, *m_pTableItemType,
                    *m_pTableItemDuration, *m_pTableItemBpm, *m_pTableItemBitrate;
    /** Pointer to WTrackTable where the current item was inserted last */
    WTrackTable *m_pTableTrack;
};

#endif
