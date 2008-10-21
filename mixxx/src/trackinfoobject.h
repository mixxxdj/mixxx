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
#include <q3memarray.h>
#include <q3valuelist.h>
#include <qmutex.h>
#include <QVector>

#include "defs.h"

class QString;
class QDomElement;
class QDomDocument;
class QDomNode;
//class WTrackTable;
//class WTrackTableItem;
class WOverview;
class ControlObject;
class BpmDetector;
class BpmReceiver;
class BpmScheme;
class TrackPlaylist;

#define NumBpmFactors 4

const float Factors[NumBpmFactors]={0.33, 0.5, 2, 0.25};

class TrackInfoObject : public QObject
{
    Q_OBJECT
public:
    /** Initialize a new track with the filename. */
    TrackInfoObject(const QString sPath, const QString sFile, BpmDetector *bpmDetector);
    /** Creates a new track given information from the xml file. */
    TrackInfoObject(const QDomNode &, BpmDetector *bpmDetector);
    ~TrackInfoObject();
    /** Returns true if the object contains valid information */
    bool isValid() const;
    int parse();
    /** Checks if the file given in m_sFilename really exists on the disc, and
        updates the m_bExists flag accordingly. Returns true if the file
        exists */
    bool checkFileExists();
    void writeToXML( QDomDocument &, QDomElement & );
    /** Insert at the values in a WTrackTable at a given row */
    //void insertInTrackTableRow(WTrackTable *pTableTrack, int iRow);
    /** Reset pointers to table cells */
    //void removeFromTrackTable();
	/** Assists in clearing the table*/
	//void clearTrackTableRow();
    /** Returns the duration in seconds */
    int getDuration() const;
    /** Returns the duration as a string: H:MM:SS */
    QString getDurationStr() const;
    /** Returns the location of the file, included path */
    QString getLocation() const;
    /** Add this TrackInfoObject instance to the BPM detection queue */
    void sendToBpmQueue();
    void sendToBpmQueue(BpmReceiver *pBpmReceiver);
    void sendToBpmQueue(BpmReceiver *pBpmReceiver, BpmScheme *pScheme);
    /** Returns BPM */
    float getBpm() const;
    /** Set BPM */
    void setBpm(float);
    /** Set BPM Correction Factors */
    void generateBpmFactors();
    /** Get BPM Correction Factors */
    void getBpmFactors(float*) const;
    /** Returns BPM as a string */
    QString getBpmStr() const;
    /** Retruns if BPM was confirmed (edited or verified manually) */
    bool getBpmConfirm() const;
    /** Set BPM confidence */
    void setBpmConfirm(bool confirm=true);
    bool getHeaderParsed() const;
    void setHeaderParsed(bool parsed = true);
    /** Returns the user comment */
    QString getComment() const;
    /** Sets the user commnet */
    void setComment(QString);
    /** Returns the file type */
    QString getType() const;
    /** Sets the type of the string */
    void setType(QString);
    /** Returns the bitrate */
    int getBitrate() const;
    /** Returns the bitrate as a string */
    QString getBitrateStr() const;
    /** Sets the bitrate */
    void setBitrate(int);
    /** Sets first beat pos */
    void setBeatFirst(float);
    /** Get first beat pos */
    float getBeatFirst() const;
    /** Retruns the length of the file in bytes */
    int getLength() const;
    /** Set sample rate */
    void setSampleRate(int iSampleRate);
    /** Get sample rate */
    int getSampleRate() const;
    /** Set number of channels */
    void setChannels(int iChannels);
    /** Get number of channels */
    int getChannels() const;
    /** Output a formatted string with all the info */
    QString getInfo() const;
    /** Set duration in seconds */
    void setDuration(int);
    /** Return title */
    QString getTitle() const;
    /** Set title */
    void setTitle(QString);
    /** Return artist */
    QString getArtist() const;
    /** Set artist */
    void setArtist(QString);
    /** Return filename */
    QString getFilename() const;
    /** Return true if the file exist */
    bool exists()  const;
    /** Return number of times the track has been played */
    int getTimesPlayed() const;
    /** Increment times played with one */
    void incTimesPlayed();
    /** Sets the filepath */
    void setFilepath(QString);
    /** Returns the score */
    int getScore() const;
    /** Returns the score as string */
    QString getScoreStr() const;
    /** Updates the score */
    void updateScore();
    /** Get id */
    int getId() const;
    /** Set id */
    void setId(int iId);
    /** Get URL for track */
    QString getURL();
    /** Set URL for track */
    void setURL(QString url);
    /** Set pointer to visual waveform data */
    void setVisualWaveform(QVector<float> *pWave);
    /** Get pointer to visual waveform data */
    QVector<float> * getVisualWaveform();

    /** Set and get this track's desired visual resample rate */
    void setVisualResampleRate(double dVisualResampleRate);
    double getVisualResampleRate();

    /** Set pointer to waveform summary -- updates UI by default */
    void setWaveSummary(Q3MemArray<char> *pWave, Q3ValueList<long> *pSegmentation, bool updateUI = true);

    /** Returns a pointer to waveform summary */
    Q3MemArray<char> *getWaveSummary();
    /** Returns a pointer to segmentation summary */
    Q3ValueList<long> *getSegmentationSummary();
    /** Return the next track as listed in WTrackTable */
    TrackInfoObject *getNext(TrackPlaylist *pPlaylist);
    /** Return the previous track as listed in WTrackTable */
    TrackInfoObject *getPrev(TrackPlaylist *pPlaylist);
    /** Set corresponding overview widget */
    void setOverviewWidget(WOverview *p);
    /** Set pointer to ControlObject holding BPM value in engine */
    void setBpmControlObject(ControlObject *p);
    /** Set pointer to ControlObject holding duration value in engine */
    void setDurationControlObject(ControlObject *p);
	QString getFilepath() const;
    /** Save the cue point (in samples... I think) */
    void setCuePoint(float cue);
    /** Get saved the cue point */
    float getCuePoint();

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
    /** URL (used in promo track) */
    QString m_sURL;
    /** Duration of track in seconds */
    int m_iDuration;
    /** Length of track in bytes */
    int m_iLength;
    /** Sample rate */
    int m_iSampleRate;
    /** Number of channels */
    int m_iChannels;
    /** Bitrate */
    int m_iBitrate;
    /** Number of times the track has been played */
    int m_iTimesPlayed;
    /** Beat per minutes (BPM) */
    float m_fBpm;
    /** Bpm Correction Factors */
    float* m_fBpmFactors;
    /** Minimum BPM range. If this is 0.0, then the config min BPM will be used */
    float m_fMinBpm;
    /** Maximum BPM range. If this is 0.0, then the config max BPM will be used */
    float m_fMaxBpm;
    /** True if BPM is confirmed */
    bool m_bBpmConfirm;
    /** True if header was parsed */
    bool m_bHeaderParsed;
    /** Position of first beat in song */
    float m_fBeatFirst;
    /** Score. Reflects the relative number of times the track has been played */
    int m_iScore;
    /** Id. Unique ID of track */
    int m_iId;
    /** Cue point in samples or something */
    float m_fCuePoint;

    /** Pointer to visual waveform info */
    QVector<float> *m_pVisualWave;
    /** Pointer to summary wave info */
    Q3MemArray<char> *m_pWave;
    /** Pointer to summary segmentation */
    Q3ValueList<long> *m_pSegmentation;
    /** WTrackTableItems are representations of the values actually shown in the WTrackTable */
    //WTrackTableItem *m_pTableItemScore, *m_pTableItemTitle, *m_pTableItemArtist, *m_pTableItemComment, *m_pTableItemType,
    //                *m_pTableItemDuration, *m_pTableItemBpm, *m_pTableItemBitrate;
    /** Pointer to WTrackTable where the current item was inserted last */
    //WTrackTable *m_pTableTrack;

    /** Maximum number of times any one track have been played */
    static int siMaxTimesPlayed;

    /** Mutex protecting access to object */
    mutable QMutex m_qMutex;
    /** Corresponding WOverview widget */
    WOverview *m_pOverviewWidget;
    /** True if object contains valid information */
    bool m_bIsValid;
    /** Pointer to ControlObject of BPM value (only set when the track is loaded in a player) */
    ControlObject *m_pControlObjectBpm;
    /** Pointer to ControlObject of duration value (only set when the track is loaded in a player) */
    ControlObject *m_pControlObjectDuration;
    int iTemp;
    double m_dVisualResampleRate;

    /** Bpm Detection Queue */
    BpmDetector *m_BpmDetector;
};

#endif
