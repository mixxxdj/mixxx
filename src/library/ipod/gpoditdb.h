/*
 * gpoditdb.h
 *
 *  Created on: 01.02.2012
 *      Author: daniel
 */

#include <QObject>

extern "C"
{
#include <gpod/itdb.h>
}

#ifndef GPODITDB_H_
#define GPODITDB_H_

class GPodItdb {
public:
    GPodItdb();
    virtual ~GPodItdb();
    bool isSupported() { return m_libGPodLoaded; };
    void parse(const QString& mount, GError **error);
    Itdb_Playlist* getFirstPlaylist();
    Itdb_Playlist* getNextPlaylist();
    Itdb_Playlist* getMasterPlaylist() { return fp_itdb_playlist_mpl(m_itdb); };
    Itdb_Playlist* getCurrentPlaylist() { return m_pCurrentPlaylist; };
    void setCurrentPlaylist(Itdb_Playlist* pl) { m_pCurrentPlaylist = pl; };

    typedef int (*itdb_free__)(Itdb_iTunesDB *itdb);
    itdb_free__ fp_itdb_free;

    typedef Itdb_Playlist* (*itdb_playlist_mpl__)(Itdb_iTunesDB *itdb);
    itdb_playlist_mpl__ fp_itdb_playlist_mpl;

    typedef Itdb_iTunesDB* (*itdb_parse__)(const gchar *mp, GError **error);
    itdb_parse__ fp_itdb_parse;

    typedef gboolean (*itdb_playlist_is_mpl__)(Itdb_Playlist *pl);
    itdb_playlist_is_mpl__ fp_itdb_playlist_is_mpl;

private:
    Itdb_iTunesDB* m_itdb;
    bool m_libGPodLoaded;
    GList* m_playlistNode;
    Itdb_Playlist *m_pCurrentPlaylist;
};

#endif /* GPODITDB_H_ */
