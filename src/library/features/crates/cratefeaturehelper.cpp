#include "library/features/crates/cratefeaturehelper.h"

#include <QInputDialog>
#include <QLineEdit>

#include "library/trackcollection.h"


CrateFeatureHelper::CrateFeatureHelper(
        CrateManager* pCrates,
        UserSettingsPointer pConfig)
        : m_pCrates(pCrates),
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
    } while (m_pCrates->storage().readCrateByName(proposedName));
    // Found an unused crate name
    return proposedName;
}

CrateId CrateFeatureHelper::createEmptyCrate() {
    const QString proposedCrateName =
            proposeNameForNewCrate(tr("New Crate"));
    Crate newCrate;
    while (!newCrate.hasName()) {
        bool ok = false;
        newCrate.parseName(
                QInputDialog::getText(
                        nullptr,
                        tr("Create New Crate"),
                        tr("Enter name for new crate:"),
                        QLineEdit::Normal,
                        proposedCrateName,
                        &ok));
        if (!ok) {
            return CrateId();
        }
        if (!newCrate.hasName()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Crate Failed"),
                    tr("A crate cannot have a blank name."));
            continue;
        }

        // select name from crateClosure join crates on id = childId group by childId having count(*) = 1;
        // check if it has parent, if not compare with names from above
        // in closure.
        // Else split path into tokens and compare to those
        if (m_pCrates->hierarchy().collectRootCrateNames().contains(newCrate.getName())) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Crate Failed"),
                    tr("A crate by that name already exists."));
            newCrate.resetName();
            continue;
        }
    }

    CrateId newCrateId;
    if (m_pCrates->insertCrate(newCrate, &newCrateId)) {
        DEBUG_ASSERT(newCrateId.isValid());
        newCrate.setId(newCrateId);
        m_pCrates->hierarchy().initClosureForCrate(newCrateId);
        m_pCrates->hierarchy().generateCratePaths(newCrate);
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

CrateId CrateFeatureHelper::createEmptySubrate(const Crate& parent) {
    const QString proposedCrateName =
            proposeNameForNewCrate(tr("New Subcrate"));
    Crate newCrate;
    while (!newCrate.hasName()) {
        bool ok = false;
        newCrate.parseName(
                QInputDialog::getText(
                        nullptr,
                        tr("Create New Crate"),
                        tr("Enter name for new crate:"),
                        QLineEdit::Normal,
                        proposedCrateName,
                        &ok));
        if (!ok) {
            return CrateId();
        }
        if (!newCrate.hasName()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Crate Failed"),
                    tr("A crate cannot have a blank name."));
            continue;
        }

        // select name from crateClosure join crates on id = childId group by childId having count(*) = 1;
        // check if it has parent, if not compare with names from above
        // in closure.
        // Else split path into tokens and compare to those
        if (m_pCrates->hierarchy().tokenizeCratePath(parent.getId()).contains(newCrate.getName())) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Crate Failed"),
                    tr("A crate by that name already exists."));
            newCrate.resetName();
            continue;
        }
    }

    CrateId newCrateId;
    if (m_pCrates->insertCrate(newCrate, &newCrateId)) {
        DEBUG_ASSERT(newCrateId.isValid());
        newCrate.setId(newCrateId);
        m_pCrates->hierarchy().initClosureForCrate(newCrateId);
        if (m_pCrates->hierarchy().insertIntoClosure(parent.getId(), newCrateId)) {
            m_pCrates->hierarchy().generateCratePaths(newCrate);
            }
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
    while (!newCrate.hasName()) {
        bool ok = false;
        newCrate.parseName(
                QInputDialog::getText(
                        nullptr,
                         tr("Duplicate Crate"),
                         tr("Enter name for new crate:"),
                         QLineEdit::Normal,
                         proposedCrateName,
                         &ok));
        if (!ok) {
            return CrateId();
        }
        if (!newCrate.hasName()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Duplicating Crate Failed"),
                    tr("A crate cannot have a blank name."));
            continue;
        }
        if (m_pCrates->storage().readCrateByName(newCrate.getName())) {
            QMessageBox::warning(
                    nullptr,
                    tr("Duplicating Crate Failed"),
                    tr("A crate by that name already exists."));
            newCrate.resetName();
            continue;
        }
    }

    CrateId newCrateId;
    if (m_pCrates->insertCrate(newCrate, &newCrateId)) {
        DEBUG_ASSERT(newCrateId.isValid());
        newCrate.setId(newCrateId);
        qDebug() << "Created new crate" << newCrate;
        QList<TrackId> trackIds;
        trackIds.reserve(
                m_pCrates->tracks().countCrateTracks(oldCrate.getId()));
        {
            CrateTrackSelectResult crateTracks(
              m_pCrates->tracks().selectCrateTracksSorted(oldCrate.getId()));
            while (crateTracks.next()) {
                trackIds.append(crateTracks.trackId());
            }
        }
        if (m_pCrates->addCrateTracks(newCrateId, trackIds)) {
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
