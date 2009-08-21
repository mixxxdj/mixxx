#include <QtDebug>

#include "library/playlistfeature.h"

PlaylistFeature::PlaylistFeature(QObject* parent)
    : LibraryFeature(parent) {
    playlists.push_back("My Rox");
    playlists.push_back("Trance");
    playlists.push_back("Technoize");
}

PlaylistFeature::~PlaylistFeature() {

}

QVariant PlaylistFeature::title() {
    return "Playlists";
}

QIcon PlaylistFeature::getIcon() {
    return QIcon(":/images/library/rhythmbox.png");
}

int PlaylistFeature::numChildren() {
    return playlists.size();
}

QVariant PlaylistFeature::child(int n) {
    return QVariant(playlists[n]);
}

void PlaylistFeature::activate() {
    qDebug("PlaylistFeature::activate()");
}

void PlaylistFeature::activateChild(int n) {
    qDebug("PlaylistFeature::activateChild(%d)", n);
    qDebug() << "Activating " << playlists[n];
}

void PlaylistFeature::onRightClick(QModelIndex index) {
}
void PlaylistFeature::onClick(QModelIndex index) {
}
