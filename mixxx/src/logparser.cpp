/***************************************************************************
                          logparser.cpp  -  read a DJO-hacks-trunk logfile
and mark tracks as played that were played in that file.  This is good for gigs
where you play more than once and exit Mixxx at some point with crashing.
                             -------------------
    begin                : Tue Aug 2 2011
    copyright            : (C) 2011 by Owen Williams
    email                : owilliams@mixxx.org
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

//TODO: this class should be able to read cue files as well

#include <QDebug>
#include <QFileInfo>
#include "logparser.h"


MixxxLogParser::MixxxLogParser(QString filename)
{
    m_sFileName = filename;    
}

MixxxLogParser::~MixxxLogParser()
{
    
}

QStringList MixxxLogParser::getPlayedTracks()
{
    QFile logfile(m_sFileName);
    QStringList played_tracks;
    QString last_played;
    
    if (!logfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Error opening log file" << m_sFileName;
        return played_tracks;
    }
    
    qDebug() << "Analyzing logfile for track played events:";
    
    //read the file, 
    QTextStream textStream(&logfile);
    while (true)
    {
        QString line = textStream.readLine();
        if (line.isNull())
        {
            break;
        } 
        else
        {
            //qDebug() << "line:" << line;
            if (line.contains("Track Played"))
            {
                last_played = extractFileName(line);
                if (last_played != NULL)
                {
                    played_tracks.append(last_played);
                    qDebug() << "played:" << last_played;
                }
            }
            else if (line.contains("Track Unplayed"))
            {
                QString unplayed = extractFileName(line);
                if (unplayed == NULL)
                    continue;
                if (unplayed == last_played && played_tracks.last() == unplayed)
                {
                    qDebug() << "unplayed: "<< unplayed;
                    played_tracks.removeLast();
                }
            }
        }
    }
    
    return played_tracks;
}

QString MixxxLogParser::extractFileName(QString line)
{
//example:
//Debug: [Main]: Track Played: "La Capria"  -  "Floating [+0Lab013]"  -  "/music/ogg/00MIXING/Netlabels/0lab/013/LaCapria_02_floating_0lab013.mp3"
    //I guess split on quote marks, take third, check file exists
    QStringList splitted = line.split("\"");
    //gets us:
    //Debug: [Main]: Track Played: 
    //La Capria
    //  -  
    //Floating [+0Lab013]
    //  -  
    ///music/ogg/00MIXING/Netlabels/0lab/013/LaCapria_02_floating_0lab013.mp3
    //
    
    //it's the second to last one, so just iterate backwards
    QStringListIterator iter(splitted);
    iter.toBack();
    while (iter.hasPrevious())
    {
        QString filename = iter.previous();
        QFileInfo possible_track(filename);
        if (possible_track.exists())
            return filename;
    }
    return NULL;
}
