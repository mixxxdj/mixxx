#ifndef MIXXXLOGPARSER_H
#define MIXXXLOGPARSER_H

#include <QString>
#include <QStringList>
#include <QFile>


class MixxxLogParser
{
    public:
        MixxxLogParser(QString filename);
        ~MixxxLogParser();
        
        QStringList getPlayedTracks();
    private:
        QString extractFileName(QString line);
        
        QString m_sFileName;
};
#endif
