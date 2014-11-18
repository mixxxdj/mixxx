
#ifndef AUDIOTAGGER_H
#define AUDIOTAGGER_H

#include <QString>
#include <QFileInfo>
#include <taglib/apetag.h>
#include <taglib/id3v2tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/mp4tag.h>

#include "util/sandbox.h"

class AudioTagger {
  public:
    AudioTagger(const QString& file, SecurityTokenPointer pToken);
    virtual ~AudioTagger();

    void setArtist(QString artist);
    void setTitle(QString title);
    void setAlbum(QString album);
    void setAlbumArtist(QString albumArtist);
    void setGenre(QString genre);
    void setComposer(QString composer);
    void setGrouping(QString grouping);
    void setYear(QString year);
    void setComment(QString comment);
    void setKey(QString key);
    void setBpm(QString bpm);
    void setTracknumber(QString tracknumber);
    bool save();

  private:
    QString m_artist;
    QString m_title;
    QString m_albumArtist;
    QString m_genre;
    QString m_composer;
    QString m_album;
    QString m_grouping;
    QString m_year;
    QString m_comment;
    QString m_key;
    QString m_bpm;
    QString m_tracknumber;

    QFileInfo m_file;
    SecurityTokenPointer m_pSecurityToken;

    /** adds or modifies the ID3v2 tag to include BPM and KEY information **/
    void addID3v2Tag(TagLib::ID3v2::Tag* id3v2);
    void addAPETag(TagLib::APE::Tag* ape);
    void addXiphComment(TagLib::Ogg::XiphComment* xiph);
    void processMP4Tag(TagLib::MP4::Tag* mp4);
};

#endif // AUDIOTAGGER_H
