// #ifndef DLGSMARTIESINFOHELPER_H
// #define DLGSMARTIESINFOHELPER_H

#pragma once

#include <QComboBox>
#include <QList>
#include <QObject>
#include <QVariant>

#include "library/dao/dao.h"
#include "library/trackset/smarties/smartiesid.h"
#include "library/trackset/smarties/smartiesstorage.h"
#include "library/trackset/tracksettablemodel.h"
//////////// db
// #include "library/dao/libraryhashdao.h"
// #include "library/dao/playlistdao.h"
// #include "library/dao/trackdao.h"
// #include "library/trackset/crate/cratestorage.h"
#include "library/trackcollection.h"
// Eve
#include "library/trackset/smarties/smartiesstorage.h"
// Eve
#include "library/trackcollectionmanager.h"
#include "preferences/usersettings.h"
#include "util/thread_affinity.h"
// class TrackCollectionManager;

class dlgSmartiesInfoHelper final : public TrackSetTableModel {
    // class dlgSmartiesInfoHelper : public QObject,
    //     public virtual /*implements*/ SqlStorage {
    Q_OBJECT

  public:
    // explicit
    dlgSmartiesInfoHelper(
            QObject* pParent,
            TrackCollectionManager* pTrackCollectionManager
            //            QObject* parent
            //            const UserSettingsPointer& pConfig
    );

    void populateComboBox(QComboBox* comboBox);
    // void test(SmartiesId smartiesId);
    void test(int smartiesId);
    //    void connectDatabase(const QSqlDatabase& database);

    // void loadSmartiesData(int smartiesId);
    QVariantList loadSmartiesData(int smartiesId);
    bool saveSmartiesData(const QVariantList& smartiesData, int smartiesId);

  private:
    // bool prepareDatabase();
};
// #endif // DLGSMARTIESINFOHELPER_H
