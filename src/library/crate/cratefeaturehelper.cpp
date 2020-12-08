#include "library/crate/cratefeaturehelper.h"

#include <QInputDialog>
#include <QLineEdit>

#include "library/trackcollection.h"
#include "moc_cratefeaturehelper.cpp"

CrateFeatureHelper::CrateFeatureHelper(
        TrackCollection* pTrackCollection,
        UserSettingsPointer pConfig)
        : m_pTrackCollection(pTrackCollection),
          m_pConfig(pConfig) {
}

QString CrateFeatureHelper::proposeNameForNewCrate(
        const QString& initialName) const {
    DEBUG_ASSERT(!initialName.isEmpty());
    QString proposedName;
    int suffixCounter = 0;
    do {
        if (suffixCounter++ > 0) {
            // Append suffix " 2", " 3", ...
            proposedName = QString("%1 %2").arg(
                    initialName, QString::number(suffixCounter));
        } else {
            proposedName = initialName;
        }
    } while (m_pTrackCollection->crates().readCrateByName(proposedName));
    // Found an unused crate name
    return proposedName;
}

CrateId CrateFeatureHelper::createEmptyCrate() {
    const QString proposedCrateName =
            proposeNameForNewCrate(tr("New Crate"));
    Crate newCrate;
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("Create New Crate"),
                        tr("Enter name for new crate:"),
                        QLineEdit::Normal,
                        proposedCrateName,
                        &ok).trimmed();
        if (!ok) {
            return CrateId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Crate Failed"),
                    tr("A crate cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->crates().readCrateByName(newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Crate Failed"),
                    tr("A crate by that name already exists."));
            continue;
        }
        newCrate.setName(std::move(newName));
        DEBUG_ASSERT(newCrate.hasName());
        break;
    }

    CrateId newCrateId;
    if (m_pTrackCollection->insertCrate(newCrate, &newCrateId)) {
        DEBUG_ASSERT(newCrateId.isValid());
        newCrate.setId(newCrateId);
        qDebug() << "Created new crate" << newCrate;
    } else {
        DEBUG_ASSERT(!newCrateId.isValid());
        qWarning() << "Failed to create new crate"
                << "->"  << newCrate.getName();
        QMessageBox::warning(
                nullptr,
                tr("Creating Crate Failed"),
                tr("An unknown error occurred while creating crate: ") + newCrate.getName());
    }
    return newCrateId;
}

CrateId CrateFeatureHelper::duplicateCrate(const Crate& oldCrate) {
    const QString proposedCrateName =
            proposeNameForNewCrate(
                    QString("%1 %2").arg(
                            oldCrate.getName(), tr("copy" , "[noun]")));
    Crate newCrate;
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                         tr("Duplicate Crate"),
                         tr("Enter name for new crate:"),
                         QLineEdit::Normal,
                         proposedCrateName,
                         &ok).trimmed();
        if (!ok) {
            return CrateId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Duplicating Crate Failed"),
                    tr("A crate cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->crates().readCrateByName(newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Duplicating Crate Failed"),
                    tr("A crate by that name already exists."));
            continue;
        }
        newCrate.setName(std::move(newName));
        DEBUG_ASSERT(newCrate.hasName());
        break;
    }

    CrateId newCrateId;
    if (m_pTrackCollection->insertCrate(newCrate, &newCrateId)) {
        DEBUG_ASSERT(newCrateId.isValid());
        newCrate.setId(newCrateId);
        qDebug() << "Created new crate" << newCrate;
        QList<TrackId> trackIds;
        trackIds.reserve(
                m_pTrackCollection->crates().countCrateTracks(oldCrate.getId()));
        {
            CrateTrackSelectResult crateTracks(
                    m_pTrackCollection->crates().selectCrateTracksSorted(oldCrate.getId()));
            while (crateTracks.next()) {
                trackIds.append(crateTracks.trackId());
            }
        }
        if (m_pTrackCollection->addCrateTracks(newCrateId, trackIds)) {
            qDebug() << "Duplicated crate"
                << oldCrate << "->" << newCrate;
        } else {
            qWarning() << "Failed to copy tracks from"
                    << oldCrate << "into" << newCrate;
        }
    } else {
        qWarning() << "Failed to duplicate crate"
                << oldCrate << "->" << newCrate.getName();
        QMessageBox::warning(
                nullptr,
                tr("Duplicating Crate Failed"),
                tr("An unknown error occurred while creating crate: ") + newCrate.getName());
    }
    return newCrateId;
}
