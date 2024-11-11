#include "library/trackset/smarties/smartiesfeaturehelper.h"

#include <QInputDialog>
#include <QLineEdit>

#include "library/trackcollection.h"
#include "library/trackset/smarties/smarties.h"
#include "library/trackset/smarties/smartiessummary.h"
#include "moc_smartiesfeaturehelper.cpp"

SmartiesFeatureHelper::SmartiesFeatureHelper(
        TrackCollection* pTrackCollection,
        UserSettingsPointer pConfig)
        : m_pTrackCollection(pTrackCollection),
          m_pConfig(pConfig) {
}

QString SmartiesFeatureHelper::proposeNameForNewSmarties(
        const QString& initialName) const {
    DEBUG_ASSERT(!initialName.isEmpty());
    QString proposedName;
    int suffixCounter = 0;
    do {
        if (suffixCounter++ > 0) {
            // Append suffix " 2", " 3", ...
            proposedName = QStringLiteral("%1 %2")
                                   .arg(initialName, QString::number(suffixCounter));
        } else {
            proposedName = initialName;
        }
    } while (m_pTrackCollection->smarties().readSmartiesByName(proposedName));
    // Found an unused smarties name
    qDebug() << "[SMARTIES] [PROPOSE NEW NAME] -> proposedName" << proposedName;
    return proposedName;
}

// EVE
SmartiesId SmartiesFeatureHelper::createEmptySmartiesFromSearch(const QString& text) {
    qDebug() << "[SMARTIES] [NEW SMARTIES FROM SEARCH]";
    Smarties newSmarties;
    const QString proposedSmartiesName =
            proposeNameForNewSmarties(tr("Search for ") + text);
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("New Smarties From Search"),
                        tr("Enter name for new smarties:"),
                        QLineEdit::Normal,
                        proposedSmartiesName,
                        &ok)
                        .trimmed();
        if (!ok) {
            return SmartiesId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Smarties Failed"),
                    tr("A smarties cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->smarties().readSmartiesByName(newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Smarties Failed"),
                    tr("A smarties by that name already exists."));
            continue;
        }
        newSmarties.setName(std::move(newName));
        DEBUG_ASSERT(newSmarties.hasName());
        break;
    }
    newSmarties.setSearchInput(std::move(text));
    newSmarties.setSearchSql(std::move(text));

    SmartiesId newSmartiesId;

    if (m_pTrackCollection->insertSmarties(newSmarties, &newSmartiesId)) {
        DEBUG_ASSERT(newSmartiesId.isValid());
        newSmarties.setId(newSmartiesId);
        qDebug() << "Created new smarties" << newSmarties;
        qDebug() << "Created new smarties ID: " << newSmartiesId;
    } else {
        DEBUG_ASSERT(!newSmartiesId.isValid());
        qWarning() << "Failed to create new smarties"
                   << "->" << newSmarties.getName();
        QMessageBox::warning(
                nullptr,
                tr("Creating Smarties Failed"),
                tr("An unknown error occurred while creating smarties: ") + newSmarties.getName());
    }
    qDebug() << "[SMARTIES] [NEW SMARTIES FROM SEARCH] -> newSmartiesId " << newSmartiesId;
    return newSmartiesId;
}
// EVE

SmartiesId SmartiesFeatureHelper::createEmptySmartiesFromUI() {
    qDebug() << "[SMARTIES] [NEW SMARTIES FROM UI] ";
    //    const QString proposedSmartiesName =
    //            proposeNameForNewSmarties(tr("New Smarties From Edit"));
    Smarties newSmarties;
    for (;;) {
        //        bool ok = false;
        // auto newName = proposedSmartiesName;
        //        auto newName =
        //                        QInputDialog::getText(
        //                        nullptr,
        //                        tr("Create New Smarties"),
        //                        tr("Enter name for new smarties:"),
        //                        QLineEdit::Normal,
        //                        proposedSmartiesName,
        //                        &ok)
        //                        .trimmed();
        //        if (!ok) {
        return SmartiesId();
    }
    //        if (newName.isEmpty()) {
    //            QMessageBox::warning(
    //                    nullptr,
    //                    tr("Creating Smarties Failed"),
    //                    tr("A smarties cannot have a blank name."));
    //            continue;
    //        }
    //        if (m_pTrackCollection->smarties().readSmartiesByName(newName)) {
    //            QMessageBox::warning(
    //                    nullptr,
    //                    tr("Creating Smarties Failed"),
    //                    tr("A smarties by that name already exists."));
    //            continue;
    //        }
    //        newSmarties.setName(std::move(newName));
    //        DEBUG_ASSERT(newSmarties.hasName());
    //        break;
    //    }

    SmartiesId newSmartiesId;
    if (m_pTrackCollection->insertSmarties(newSmarties, &newSmartiesId)) {
        DEBUG_ASSERT(newSmartiesId.isValid());
        newSmarties.setId(newSmartiesId);
        qDebug() << "Created new smarties" << newSmarties;
    } else {
        DEBUG_ASSERT(!newSmartiesId.isValid());
        qWarning() << "Failed to create new smarties"
                   << "->" << newSmarties.getName();
        //        QMessageBox::warning(
        //                nullptr,
        //                tr("Creating Smarties Failed"),
        //                tr("An unknown error occurred while creating smarties:
        //                ") + newSmarties.getName());
    }
    qDebug() << "[SMARTIES] [NEW SMARTIES FROM UI] -> newSmartiesId " << newSmartiesId;
    return newSmartiesId;
}

SmartiesId SmartiesFeatureHelper::createEmptySmarties() {
    qDebug() << "[SMARTIES] [NEW EMPTY SMARTIES] ";
    const QString proposedSmartiesName =
            proposeNameForNewSmarties(tr("New Smarties"));
    Smarties newSmarties;
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("Create New Smarties"),
                        tr("Enter name for new smarties:"),
                        QLineEdit::Normal,
                        proposedSmartiesName,
                        &ok)
                        .trimmed();
        if (!ok) {
            return SmartiesId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Smarties Failed"),
                    tr("A smarties cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->smarties().readSmartiesByName(newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Smarties Failed"),
                    tr("A smarties by that name already exists."));
            continue;
        }
        newSmarties.setName(std::move(newName));
        DEBUG_ASSERT(newSmarties.hasName());
        break;
    }

    SmartiesId newSmartiesId;
    if (m_pTrackCollection->insertSmarties(newSmarties, &newSmartiesId)) {
        DEBUG_ASSERT(newSmartiesId.isValid());
        newSmarties.setId(newSmartiesId);
        qDebug() << "Created new smarties" << newSmarties;
    } else {
        DEBUG_ASSERT(!newSmartiesId.isValid());
        qWarning() << "Failed to create new smarties"
                   << "->" << newSmarties.getName();
        QMessageBox::warning(
                nullptr,
                tr("Creating Smarties Failed"),
                tr("An unknown error occurred while creating smarties: ") + newSmarties.getName());
    }
    qDebug() << "[SMARTIES] [NEW EMPTY SMARTIES] -> newSmartiesId " << newSmartiesId;
    return newSmartiesId;
}

SmartiesId SmartiesFeatureHelper::duplicateSmarties(const Smarties& oldSmarties) {
    qDebug() << "[SMARTIES] [DUPLICATE SMARTIES] -> START";
    const QString proposedSmartiesName =
            proposeNameForNewSmarties(
                    QStringLiteral("%1 %2")
                            .arg(oldSmarties.getName(), tr("copy", "//:")));

    QString newSearchInput = oldSmarties.getSearchInput();
    qDebug() << "[SMARTIES] [DUPLICATE] -> old searchInput" << newSearchInput;
    QString newSearchSql = oldSmarties.getSearchSql();
    qDebug() << "[SMARTIES] [DUPLICATE] -> old searchSql" << newSearchSql;
    QString newCondition1Field = oldSmarties.getCondition1Field();
    qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition1Field" << newCondition1Field;
    QString newCondition2Field = oldSmarties.getCondition2Field();
    qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition2Field" << newCondition2Field;
    QString newCondition3Field = oldSmarties.getCondition3Field();
    qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition3Field" << newCondition3Field;
    QString newCondition4Field = oldSmarties.getCondition4Field();
    QString newCondition5Field = oldSmarties.getCondition5Field();
    QString newCondition6Field = oldSmarties.getCondition6Field();
    QString newCondition7Field = oldSmarties.getCondition7Field();
    QString newCondition8Field = oldSmarties.getCondition8Field();
    QString newCondition9Field = oldSmarties.getCondition9Field();
    QString newCondition10Field = oldSmarties.getCondition10Field();
    QString newCondition11Field = oldSmarties.getCondition11Field();
    QString newCondition12Field = oldSmarties.getCondition12Field();

    QString newCondition1Operator = oldSmarties.getCondition1Operator();
    qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition1Operator" << newCondition1Operator;
    QString newCondition2Operator = oldSmarties.getCondition2Operator();
    qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition2Operator" << newCondition2Operator;
    QString newCondition3Operator = oldSmarties.getCondition3Operator();
    qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition3Operator" << newCondition3Operator;
    QString newCondition4Operator = oldSmarties.getCondition4Operator();
    QString newCondition5Operator = oldSmarties.getCondition5Operator();
    QString newCondition6Operator = oldSmarties.getCondition6Operator();
    QString newCondition7Operator = oldSmarties.getCondition7Operator();
    QString newCondition8Operator = oldSmarties.getCondition8Operator();
    QString newCondition9Operator = oldSmarties.getCondition9Operator();
    QString newCondition10Operator = oldSmarties.getCondition10Operator();
    QString newCondition11Operator = oldSmarties.getCondition11Operator();
    QString newCondition12Operator = oldSmarties.getCondition12Operator();

    QString newCondition1Value = oldSmarties.getCondition1Value();
    qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition1Value" << newCondition1Value;
    QString newCondition2Value = oldSmarties.getCondition2Value();
    qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition2Value" << newCondition2Value;
    QString newCondition3Value = oldSmarties.getCondition3Value();
    qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition3Value" << newCondition2Value;
    QString newCondition4Value = oldSmarties.getCondition4Value();
    QString newCondition5Value = oldSmarties.getCondition5Value();
    QString newCondition6Value = oldSmarties.getCondition6Value();
    QString newCondition7Value = oldSmarties.getCondition7Value();
    QString newCondition8Value = oldSmarties.getCondition8Value();
    QString newCondition9Value = oldSmarties.getCondition9Value();
    QString newCondition10Value = oldSmarties.getCondition10Value();
    QString newCondition11Value = oldSmarties.getCondition11Value();
    QString newCondition12Value = oldSmarties.getCondition12Value();

    QString newCondition1Combiner = oldSmarties.getCondition1Combiner();
    qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition1Combiner" << newCondition1Combiner;
    QString newCondition2Combiner = oldSmarties.getCondition2Combiner();
    qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition2Combiner" << newCondition2Combiner;
    QString newCondition3Combiner = oldSmarties.getCondition3Combiner();
    qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition3Combiner" << newCondition3Combiner;
    QString newCondition4Combiner = oldSmarties.getCondition4Combiner();
    QString newCondition5Combiner = oldSmarties.getCondition5Combiner();
    QString newCondition6Combiner = oldSmarties.getCondition6Combiner();
    QString newCondition7Combiner = oldSmarties.getCondition7Combiner();
    QString newCondition8Combiner = oldSmarties.getCondition8Combiner();
    QString newCondition9Combiner = oldSmarties.getCondition9Combiner();
    QString newCondition10Combiner = oldSmarties.getCondition10Combiner();
    QString newCondition11Combiner = oldSmarties.getCondition11Combiner();
    QString newCondition12Combiner = oldSmarties.getCondition12Combiner();

    Smarties newSmarties;
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("Duplicate Smarties"),
                        tr("Enter name for new smarties:"),
                        QLineEdit::Normal,
                        proposedSmartiesName,
                        &ok)
                        .trimmed();
        if (!ok) {
            return SmartiesId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Duplicating Smarties Failed"),
                    tr("A smarties cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->smarties().readSmartiesByName(newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Duplicating Smarties Failed"),
                    tr("A smarties by that name already exists."));
            continue;
        }
        newSmarties.setName(std::move(newName));
        newSmarties.setSearchInput(newSearchInput);
        newSmarties.setSearchSql(newSearchSql);
        newSmarties.setCondition1Field(newCondition1Field);
        newSmarties.setCondition2Field(newCondition2Field);
        newSmarties.setCondition3Field(newCondition3Field);
        newSmarties.setCondition4Field(newCondition4Field);
        newSmarties.setCondition5Field(newCondition5Field);
        newSmarties.setCondition6Field(newCondition6Field);
        newSmarties.setCondition7Field(newCondition7Field);
        newSmarties.setCondition8Field(newCondition8Field);
        newSmarties.setCondition9Field(newCondition9Field);
        newSmarties.setCondition10Field(newCondition10Field);
        newSmarties.setCondition11Field(newCondition11Field);
        newSmarties.setCondition12Field(newCondition12Field);
        newSmarties.setCondition1Operator(newCondition1Operator);
        newSmarties.setCondition2Operator(newCondition2Operator);
        newSmarties.setCondition3Operator(newCondition3Operator);
        newSmarties.setCondition4Operator(newCondition4Operator);
        newSmarties.setCondition5Operator(newCondition5Operator);
        newSmarties.setCondition6Operator(newCondition6Operator);
        newSmarties.setCondition7Operator(newCondition7Operator);
        newSmarties.setCondition8Operator(newCondition8Operator);
        newSmarties.setCondition9Operator(newCondition9Operator);
        newSmarties.setCondition10Operator(newCondition10Operator);
        newSmarties.setCondition11Operator(newCondition11Operator);
        newSmarties.setCondition12Operator(newCondition12Operator);
        newSmarties.setCondition1Value(newCondition1Value);
        newSmarties.setCondition2Value(newCondition2Value);
        newSmarties.setCondition3Value(newCondition3Value);
        newSmarties.setCondition4Value(newCondition4Value);
        newSmarties.setCondition5Value(newCondition5Value);
        newSmarties.setCondition6Value(newCondition6Value);
        newSmarties.setCondition7Value(newCondition7Value);
        newSmarties.setCondition8Value(newCondition8Value);
        newSmarties.setCondition9Value(newCondition9Value);
        newSmarties.setCondition10Value(newCondition10Value);
        newSmarties.setCondition11Value(newCondition11Value);
        newSmarties.setCondition12Value(newCondition12Value);
        newSmarties.setCondition1Combiner(newCondition1Combiner);
        newSmarties.setCondition2Combiner(newCondition2Combiner);
        newSmarties.setCondition3Combiner(newCondition3Combiner);
        newSmarties.setCondition4Combiner(newCondition4Combiner);
        newSmarties.setCondition5Combiner(newCondition5Combiner);
        newSmarties.setCondition6Combiner(newCondition6Combiner);
        newSmarties.setCondition7Combiner(newCondition7Combiner);
        newSmarties.setCondition8Combiner(newCondition8Combiner);
        newSmarties.setCondition9Combiner(newCondition9Combiner);
        newSmarties.setCondition10Combiner(newCondition10Combiner);
        newSmarties.setCondition11Combiner(newCondition11Combiner);
        newSmarties.setCondition12Combiner(newCondition12Combiner);

        DEBUG_ASSERT(newSmarties.hasName());
        break;
    }

    SmartiesId newSmartiesId;
    if (m_pTrackCollection->insertSmarties(newSmarties, &newSmartiesId)) {
        DEBUG_ASSERT(newSmartiesId.isValid());
        newSmarties.setId(newSmartiesId);

        qDebug() << "[SMARTIES] [DUPLICATE SMARTIES] -> Created new smarties" << newSmarties;
        QList<TrackId> trackIds;
        trackIds.reserve(
                m_pTrackCollection->smarties().countSmartiesTracks(oldSmarties.getId()));
        {
            SmartiesTrackSelectResult smartiesTracks(
                    m_pTrackCollection->smarties().selectSmartiesTracksSorted(oldSmarties.getId()));
            while (smartiesTracks.next()) {
                trackIds.append(smartiesTracks.trackId());
            }
        }
        if (m_pTrackCollection->addSmartiesTracks(newSmartiesId, trackIds)) {
            qDebug() << "[SMARTIES] [DUPLICATE SMARTIES] Duplicated smarties -> "
                     << oldSmarties << "->" << newSmarties;
        } else {
            qWarning() << "Failed to copy tracks from "
                       << oldSmarties << "into" << newSmarties;
        }
    } else {
        qWarning() << "Failed to duplicate smarties "
                   << oldSmarties << "->" << newSmarties.getName();
        QMessageBox::warning(
                nullptr,
                tr("Duplicating Smarties Failed"),
                tr("An unknown error occurred while creating smarties: ") + newSmarties.getName());
    }
    qDebug() << "[SMARTIES] [DUPLICATE SMARTIES] -> END -> newSmartiesId " << newSmartiesId;
    return newSmartiesId;
}
