#include "dlgsmartiesactions.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include "library/dao/trackschema.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/smarties/smarties.h"
// #include "moc_dlgsmartiesactions.cpp"
#include "track/track.h"
#include "util/db/fwdsqlquery.h"

// EVE
#include "library/trackset/smarties/smartiesschema.h"
#include "library/trackset/smarties/smartiesstorage.h"
#include "util/db/fwdsqlqueryselectresult.cpp"
#include "util/logger.h"
// EVE

// namespace {
// } // namespace

dlgSmartiesActions::dlgSmartiesActions(
        QObject* pParent,
        TrackCollectionManager* pTrackCollectionManager)
        : TrackSetTableModel(
                  pParent,
                  pTrackCollectionManager,
                  "mixxx.db.model.smarties") {
}
