#include "tracksettablemodel.h"

TrackSetTableModel::TrackSetTableModel(QObject* parent, TrackCollectionManager* pTrackCollectionManager, const char* settingsNamespace)
        : BaseSqlTableModel(parent, pTrackCollectionManager, settingsNamespace) {
}
