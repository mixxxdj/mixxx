#include "library/trackset/smarties/dlgsmartiesinfohelper.h"

#include <QCombobox>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

// #include "library/dao/trackschema.h"
// #include "library/trackcollection.h"
// #include "library/trackcollectionmanager.h"
#include "library/trackset/smarties/smarties.h"
#include "moc_dlgsmartiesinfohelper.cpp"
// #include "track/track.h"
#include "util/db/fwdsqlquery.h"

// EVE
#include "library/trackset/smarties/smartiesschema.h"
#include "library/trackset/smarties/smartiesstorage.h"
#include "library/trackset/smarties/ui_dlgsmartiesinfo.h"
#include "util/db/fwdsqlqueryselectresult.cpp"
#include "util/logger.h"

//// db
// #include "track/globaltrackcache.h"
#include "util/db/sqltransaction.h"
// EVE

namespace {
} // namespace

dlgSmartiesInfoHelper::dlgSmartiesInfoHelper(
        QObject* pParent,
        TrackCollectionManager* pTrackCollectionManager)
        : TrackSetTableModel(
                  pParent,
                  pTrackCollectionManager,
                  "mixxx.db.model.smarties")
//         ,
//             m_pUI(new Ui::dlgSmartiesInfo)
{
    //    TrackCollection::connectDatabase(const QSqlDatabase& database);
    //    loadSmartiesData(1); // Load data when the actions class is created
    //        m_pUI->setupUi(this);

    //    m_pUI->setupUi(this);
}

QVariantList dlgSmartiesInfoHelper::loadSmartiesData(int smartiesId) {
    // TrackCollection::connectDatabase(const QSqlDatabase& database);
    // void dlgSmartiesInfoHelper::loadSmartiesData(int smartiesId) {
    // QVariant dlgSmartiesActions::getSmartiesData(int smartiesId) const {
    TrackCollection::connectDatabase(const QSqlDatabase& database);
    //    void dlgSmartiesInfoHelper::loadSmartiesData() {
    //    connectDatabase(database)
    QVariantList smartiesData;
    qDebug() << "dlgSmartiesInfoHelper::loadSmartiesData() started ";
    // m_smartiesList.clear(); // Clear existing data
    QSqlQuery* query = new QSqlQuery(m_database);
    QSqlQuery query("SELECT * FROM smarties");
    if (query.exec()) {
        while (query.next()) {
            QVariantList data;
            smartiesData.append(query.value(0).toString()); // id
            smartiesData.append(query.value(1).toString()); // name
            smartiesData.append(query.value(2).toInt());    // count
            smartiesData.append(query.value(3).toBool());   // show
            smartiesData.append(query.value(4).toBool());   // locked
            smartiesData.append(query.value(5).toBool());   // autoDJ
            smartiesData.append(query.value(6).toString()); // search_input
            smartiesData.append(query.value(7).toString()); // search_sql

            smartiesData.append(query.value(8).toString());  // condition 1 field
            smartiesData.append(query.value(9).toString());  // condition 1 operator
            smartiesData.append(query.value(10).toString()); // condition 1 value
            smartiesData.append(query.value(11).toString()); // condition 1 combiner

            smartiesData.append(query.value(12).toString()); // condition 2 field
            smartiesData.append(query.value(13).toString()); // condition 2 operator
            smartiesData.append(query.value(14).toString()); // condition 2 value
            smartiesData.append(query.value(15).toString()); // condition 2 combiner

            smartiesData.append(query.value(16).toString()); // condition 3 field
            smartiesData.append(query.value(17).toString()); // condition 3 operator
            smartiesData.append(query.value(18).toString()); // condition 3 value
            smartiesData.append(query.value(19).toString()); // condition 3 combiner

            smartiesData.append(query.value(20).toString()); // condition 4 field
            smartiesData.append(query.value(21).toString()); // condition 4 operator
            smartiesData.append(query.value(22).toString()); // condition 4 value
            smartiesData.append(query.value(23).toString()); // condition 4 combiner

            smartiesData.append(query.value(24).toString()); // condition 5 field
            smartiesData.append(query.value(25).toString()); // condition 5 operator
            smartiesData.append(query.value(26).toString()); // condition 5 value
            smartiesData.append(query.value(27).toString()); // condition 5 combiner

            smartiesData.append(query.value(28).toString()); // condition 6 field
            smartiesData.append(query.value(29).toString()); // condition 6 operator
            smartiesData.append(query.value(30).toString()); // condition 6 value
            smartiesData.append(query.value(31).toString()); // condition 6 combiner

            smartiesData.append(query.value(32).toString()); // condition 7 field
            smartiesData.append(query.value(33).toString()); // condition 7 operator
            smartiesData.append(query.value(34).toString()); // condition 7 value
            smartiesData.append(query.value(35).toString()); // condition 7 combiner

            smartiesData.append(query.value(36).toString()); // condition 8 field
            smartiesData.append(query.value(37).toString()); // condition 8 operator
            smartiesData.append(query.value(38).toString()); // condition 8 value
            smartiesData.append(query.value(39).toString()); // condition 8 combiner

            smartiesData.append(query.value(40).toString()); // condition 9 field
            smartiesData.append(query.value(41).toString()); // condition 9 operator
            smartiesData.append(query.value(42).toString()); // condition 9 value
            smartiesData.append(query.value(43).toString()); // condition 9 combiner

            smartiesData.append(query.value(44).toString()); // condition 10 field
            smartiesData.append(query.value(45).toString()); // condition 10 operator
            smartiesData.append(query.value(46).toString()); // condition 10 value
            smartiesData.append(query.value(47).toString()); // condition 10 combiner

            smartiesData.append(query.value(48).toString()); // condition 11 field
            smartiesData.append(query.value(49).toString()); // condition 11 operator
            smartiesData.append(query.value(50).toString()); // condition 11 value
            smartiesData.append(query.value(51).toString()); // condition 11 combiner

            smartiesData.append(query.value(52).toString()); // condition 12 field
            smartiesData.append(query.value(53).toString()); // condition 12 operator
            smartiesData.append(query.value(54).toString()); // condition 12 value
            smartiesData.append(query.value(55).toString()); // condition 12 combiner
            qDebug() << "dlgSmartiesInfoHelper::loadSmartiesData() stored "
                     << query.value(0).toString() << " - "
                     << query.value(1).toString();
            m_smartiesList.append(data);
        }
    }
    qDebug() << "dlgSmartiesInfoHelper::loadSmartiesData() finished ";
}

// QList<QVariant> dlgSmartiesInfoHelper::loadSmarties() {
//     QList<QVariant> smartiesList;
//     QSqlQuery query("SELECT * FROM smarties"); // Adjust query as necessary
//
//     if (query.exec()) {
//         while (query.next()) {
//             QVariant smarties;
//             // Populate smartie with data from the query result
//             smarties = query.value("name").toString(); // Example, adjust fields accordingly
//             smartiesList.append(smarties);
//         }
//     } else {
//         qDebug() << "Error loading smarties:" << query.lastError().text();
//     }
//     return smartiesList;
// }

bool dlgSmartiesInfoHelper::saveSmartiesData(const QVariantList& smartiesData, int smartiesId) {
    QSqlQuery* query = new QSqlQuery(m_database);
    QSqlQuery query;
    query.prepare(
            "UPDATE smarties SET "
            "name = :name, count = :count, show = :show, locked = :locked, "
            "autodj_source = :autodj_source, "
            "search_input = :search_input, search_sql = :search_sql, "
            "condition1_field = :condition1_field, condition1_operator = "
            ":condition1_operator, "
            "condition1_value = :condition1_value, condition1_combiner = "
            ":condition1_combiner, "
            "condition2_field = :condition2_field, condition2_operator = "
            ":condition2_operator, "
            "condition2_value = :condition2_value, condition2_combiner = "
            ":condition2_combiner, "
            "condition3_field = :condition3_field, condition3_operator = "
            ":condition3_operator, "
            "condition3_value = :condition3_value, condition3_combiner = "
            ":condition3_combiner, "
            "condition4_field = :condition4_field, condition4_operator = "
            ":condition4_operator, "
            "condition4_value = :condition4_value, condition4_combiner = "
            ":condition4_combiner, "
            "condition5_field = :condition5_field, condition5_operator = "
            ":condition5_operator, "
            "condition5_value = :condition5_value, condition5_combiner = "
            ":condition5_combiner, "
            "condition6_field = :condition6_field, condition6_operator = "
            ":condition6_operator, "
            "condition6_value = :condition6_value, condition6_combiner = "
            ":condition6_combiner, "
            "condition7_field = :condition7_field, condition7_operator = "
            ":condition7_operator, "
            "condition7_value = :condition7_value, condition7_combiner = "
            ":condition7_combiner, "
            "condition8_field = :condition8_field, condition8_operator = "
            ":condition8_operator, "
            "condition8_value = :condition8_value, condition8_combiner = "
            ":condition8_combiner, "
            "condition9_field = :condition9_field, condition9_operator = "
            ":condition9_operator, "
            "condition9_value = :condition9_value, condition9_combiner = "
            ":condition9_combiner, "
            "condition10_field = :condition10_field, condition10_operator = "
            ":condition10_operator, "
            "condition10_value = :condition10_value, condition10_combiner = "
            ":condition10_combiner, "
            "condition11_field = :condition11_field, condition11_operator = "
            ":condition11_operator, "
            "condition11_value = :condition11_value, condition11_combiner = "
            ":condition11_combiner, "
            "condition12_field = :condition12_field, condition12_operator = "
            ":condition12_operator, "
            "condition12_value = :condition12_value, condition12_combiner = "
            ":condition12_combiner "
            "WHERE id = :id");

    for (int i = 0; i < m_smartiesList.size(); ++i) {
        QVariantList rowData = m_smartiesList[i].toList();

        query.bindValue(":name", rowData[0]);
        query.bindValue(":count", rowData[1]);
        query.bindValue(":show", rowData[2]);
        query.bindValue(":locked", rowData[3]);
        query.bindValue(":autodj_source", rowData[4]);
        query.bindValue(":search_input", rowData[5]);
        query.bindValue(":search_sql", rowData[6]);

        // Bind each condition field
        int dataIndex = 7;
        for (int conditionIndex = 1; conditionIndex <= 12; ++conditionIndex) {
            query.bindValue(QString(":condition%1_field").arg(conditionIndex),
                    rowData[dataIndex++]);
            query.bindValue(
                    QString(":condition%1_operator").arg(conditionIndex),
                    rowData[dataIndex++]);
            query.bindValue(QString(":condition%1_value").arg(conditionIndex),
                    rowData[dataIndex++]);
            query.bindValue(
                    QString(":condition%1_combiner").arg(conditionIndex),
                    rowData[dataIndex++]);
        }

        // Bind the ID for the WHERE clause
        query.bindValue(":id", rowData[0]); // Assuming id is the first element in rowData

        // Execute the query for each smartie row
        if (!query.exec()) {
            qDebug() << "Failed to update smarties table:" << query.lastError();
        }
    }
    return true;
}

// void dlgSmartiesInfoHelper::saveSmarties(const QVariant& smartiesData) {
//     QSqlQuery* query = new QSqlQuery(m_database);
//     QSqlQuery query;
//     query.prepare("INSERT INTO smarties (name, count) VALUES (?, ?)"); //
//     Adjust query as necessary

// Assuming smartiesData is structured appropriately
//    query.addBindValue(smartiesData.toString()); // Adjust based on actual data structure
//    query.addBindValue(0);                       // Default count, adjust as necessary

//    if (!query.exec()) {
//        qDebug() << "Error saving smarties:" << query.lastError().text();
//    }
//}

void dlgSmartiesInfoHelper::populateComboBox(QComboBox* comboBox) {
    //    comboBox->clear();
    //    QSqlQuery* query = new QSqlQuery(m_database);
    //    QSqlQuery query("SELECT artist FROM library"); // Adjust as necessary

    //    if (query.exec()) {
    //        while (query.next()) {
    //            QString artist = query.value("artist").toString(); // Assuming
    //            you want to populate with artists comboBox->addItem(artist);
    //        }
    //    } else {
    //        qDebug() << "Error populating combo box:" <<
    //        query.lastError().text();
    //    }
}

QVariantList dlgSmartiesInfoHelper::getSmartiesData(int smartiesId) const {
    // Assuming smartiesId corresponds to the index in the list
    if (smartiesId >= 0 && smartiesId < m_smartiesList.size()) {
        return m_smartiesList[smartiesId];
    }
    return QVariantList(); // Return empty QVariantList if invalid smartiesId
}

// void dlgSmartiesInfoHelper::init(SmartiesId smartiesId) {
//     qDebug() << "INIT: currentSmartiesId: " << smartiesId.toString();
//     QSqlQuery* queryGetSearchSqlAndName = new QSqlQuery(m_database);
//     queryGetSearchSqlAndName->prepare("SELECT id, name, count, show, locked,
//     autodj_source, search_input, search_sql from smarties where id=:id");
//     queryGetSearchSqlAndName->addBindValue(smartiesId.toVariant().toString());

//    queryGetSearchSqlAndName->exec();
//    queryGetSearchSqlAndName->next();
//    QString searchID = queryGetSearchSqlAndName->value(0).toString();
//    QString searchName = queryGetSearchSqlAndName->value(1).toString();
//    QString searchCount = queryGetSearchSqlAndName->value(2).toString();
//    QString searchShow = queryGetSearchSqlAndName->value(3).toString();
//    QString searchLocked = queryGetSearchSqlAndName->value(4).toString();
//    QString searchAutoDJSource = queryGetSearchSqlAndName->value(5).toString();
//    QString searchInput = queryGetSearchSqlAndName->value(6).toString();
//    QString searchSQL = queryGetSearchSqlAndName->value(7).toString();

// ui->lineEditID->setText(query.value("id").toString());
//    m_pUI->lineEditID->setText(queryGetSearchSqlAndName->value("id").toString());
//    m_pUI->lineEditName->setText(queryGetSearchSqlAndName->value("name").toString());
//    m_pUI->spinBoxCount->setValue(queryGetSearchSqlAndName->value("count").toInt());
//    m_pUI->checkBoxShow->setChecked(queryGetSearchSqlAndName->value("show").toBool());
//    m_pUI->buttonLock->setText(queryGetSearchSqlAndName->value("locked").toBool()
//    ? "Unlock" : "Lock");
//    m_pUI->checkBoxAutoDJ->setChecked(queryGetSearchSqlAndName->value("autodj").toBool());
//    m_pUI->lineEditSearchInput->setText(queryGetSearchSqlAndName->value("search_input").toString());
//    m_pUI->lineEditSearchSQL->setText(queryGetSearchSqlAndName->value("search_sql").toString());

//    qDebug() << "queryGetSearchValue " << queryGetSearchSqlAndName;
//    qDebug() << "searchID " << searchID;
//    qDebug() << "searchName " << searchName;
//    qDebug() << "searchCount " << searchCount;
//    qDebug() << "searchShow " << searchShow;
//    qDebug() << "searchLocked " << searchLocked;
//    qDebug() << "searchDJSource " << searchAutoDJSource;
//    qDebug() << "searchInput " << searchInput;
//    qDebug() << "searchSQL " << searchSQL;

//    queryGetSearchSqlAndName->clear();

//}

// void dlgSmartiesInfo::test(SmartiesId smartiesId) {
void dlgSmartiesInfo::test(int smartiesId) {
    qDebug() << "INIT: currentSmartiesId: " << smartiesId.toString();
    QSqlQuery* queryGetSearchSqlAndName = new QSqlQuery(m_database);
    queryGetSearchSqlAndName->prepare(
            "SELECT id, name, count, show, locked, autodj_source, "
            "search_input, search_sql from smarties where id=:id");
    queryGetSearchSqlAndName->addBindValue(smartiesId.toVariant().toString());

    queryGetSearchSqlAndName->exec();
    queryGetSearchSqlAndName->next();
    QString searchID = queryGetSearchSqlAndName->value(0).toString();
    QString searchName = queryGetSearchSqlAndName->value(1).toString();
    QString searchCount = queryGetSearchSqlAndName->value(2).toString();
    QString searchShow = queryGetSearchSqlAndName->value(3).toString();
    QString searchLocked = queryGetSearchSqlAndName->value(4).toString();
    QString searchAutoDJSource = queryGetSearchSqlAndName->value(5).toString();
    QString searchInput = queryGetSearchSqlAndName->value(6).toString();
    QString searchSQL = queryGetSearchSqlAndName->value(7).toString();

    // ui->lineEditID->setText(query.value("id").toString());
    //    m_pUI->lineEditID->setText(queryGetSearchSqlAndName->value("id").toString());
    //    m_pUI->lineEditName->setText(queryGetSearchSqlAndName->value("name").toString());
    //    m_pUI->spinBoxCount->setValue(queryGetSearchSqlAndName->value("count").toInt());
    //    m_pUI->checkBoxShow->setChecked(queryGetSearchSqlAndName->value("show").toBool());
    //    m_pUI->buttonLock->setText(queryGetSearchSqlAndName->value("locked").toBool()
    //    ? "Unlock" : "Lock");
    //    m_pUI->checkBoxAutoDJ->setChecked(queryGetSearchSqlAndName->value("autodj").toBool());
    //    m_pUI->lineEditSearchInput->setText(queryGetSearchSqlAndName->value("search_input").toString());
    //    m_pUI->lineEditSearchSQL->setText(queryGetSearchSqlAndName->value("search_sql").toString());

    qDebug() << "queryGetSearchValue " << queryGetSearchSqlAndName;
    qDebug() << "searchID " << searchID;
    qDebug() << "searchName " << searchName;
    qDebug() << "searchCount " << searchCount;
    qDebug() << "searchShow " << searchShow;
    qDebug() << "searchLocked " << searchLocked;
    qDebug() << "searchDJSource " << searchAutoDJSource;
    qDebug() << "searchInput " << searchInput;
    qDebug() << "searchSQL " << searchSQL;

    queryGetSearchSqlAndName->clear();
}
}
