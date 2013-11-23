/*
 * GPodItdb.cpp
 *
 *  Created on: 01.02.2012
 *      Author: Daniel Schürmann
 */

#include <QLibrary>
#include <QtDebug>
#include <QDir>
#include <QMessageBox>

#include "gpoditdb.h"


GPodItdb::GPodItdb() :
        m_itdb(NULL),
        m_libGPodLoaded(true) {

    // Load shared library
    QLibrary libGPod("libgpod");

    fp_itdb_free = (itdb_free__)libGPod.resolve("itdb_free");
    if(!fp_itdb_free) m_libGPodLoaded = false;

    fp_itdb_playlist_mpl = (itdb_playlist_mpl__)libGPod.resolve("itdb_playlist_mpl");
    if(!fp_itdb_playlist_mpl) m_libGPodLoaded = false;

    fp_itdb_parse = (itdb_parse__)libGPod.resolve("itdb_parse");
    if(!fp_itdb_parse) m_libGPodLoaded = false;

    qDebug() << "GPodItdb: try to resolve libgpod functions: " << (m_libGPodLoaded?"success":"failed");
}

GPodItdb::~GPodItdb() {
    if (m_itdb) {
        fp_itdb_free(m_itdb);
    }
}

void GPodItdb::parse(const QString& mount, GError **error) {

    if (m_itdb) {
        fp_itdb_free(m_itdb);
        m_itdb = NULL;
    }
    m_itdb = fp_itdb_parse(QDir::toNativeSeparators(mount).toLocal8Bit(), error);
}

Itdb_Playlist* GPodItdb::getFirstPlaylist() {
    m_playlistNode = g_list_first(m_itdb->playlists);
    if (m_playlistNode) {
        return (Itdb_Playlist*)m_playlistNode->data;
    }
    return NULL;
}

Itdb_Playlist* GPodItdb::getNextPlaylist() {
    m_playlistNode = g_list_next(m_playlistNode);
    if (m_playlistNode) {
        return (Itdb_Playlist*)m_playlistNode->data;
    }
    return NULL;
}
