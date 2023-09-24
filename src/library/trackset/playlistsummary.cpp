#include "library/trackset/playlistsummary.h"

#include "moc_playlistsummary.cpp"

PlaylistSummaryWrapper::PlaylistSummaryWrapper(PlaylistSummary& summary)
        : m_summary(summary) {
}

PlaylistSummaryWrapper::~PlaylistSummaryWrapper() {
}
