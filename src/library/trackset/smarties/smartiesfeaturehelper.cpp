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
    return proposedName;
}

// SmartiesId SmartiesFeatureHelper::createEmptySmartiesFromSearch(QString NewSmartiesName) {
//     Smarties newSmarties;
//     auto newName = NewSmartiesName;
//     bool ok = true;
//     if (!ok) {
//         return SmartiesId();
//     }

//    newSmarties.setName(std::move(newName));
//    SmartiesId newSmartiesId;
//    newSmarties.setId(newSmartiesId);
//    qDebug() << "Created new smarties" << newSmarties;
//    return newSmartiesId;
//}

SmartiesId SmartiesFeatureHelper::createEmptySmarties() {
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
    return newSmartiesId;
}

SmartiesId SmartiesFeatureHelper::duplicateSmarties(const Smarties& oldSmarties) {
    const QString proposedSmartiesName =
            proposeNameForNewSmarties(
                    QStringLiteral("%1 %2")
                            .arg(oldSmarties.getName(), tr("copy", "//:")));
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
        DEBUG_ASSERT(newSmarties.hasName());
        break;
    }

    SmartiesId newSmartiesId;
    if (m_pTrackCollection->insertSmarties(newSmarties, &newSmartiesId)) {
        DEBUG_ASSERT(newSmartiesId.isValid());
        newSmarties.setId(newSmartiesId);
        qDebug() << "Created new smarties" << newSmarties;
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
            qDebug() << "Duplicated smarties"
                     << oldSmarties << "->" << newSmarties;
        } else {
            qWarning() << "Failed to copy tracks from"
                       << oldSmarties << "into" << newSmarties;
        }
    } else {
        qWarning() << "Failed to duplicate smarties"
                   << oldSmarties << "->" << newSmarties.getName();
        QMessageBox::warning(
                nullptr,
                tr("Duplicating Smarties Failed"),
                tr("An unknown error occurred while creating smarties: ") + newSmarties.getName());
    }
    return newSmartiesId;
}
