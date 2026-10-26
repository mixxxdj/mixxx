#include "library/libraryfeature.h"

#include <QStandardPaths>

#include "library/library.h"
#include "moc_libraryfeature.cpp"
#include "util/logger.h"

// KEEP THIS cpp file to tell scons that moc should be called on the class!!!
// The reason for this is that LibraryFeature uses slots/signals and for this
// to work the code has to be precompiles by moc

namespace {

const mixxx::Logger kLogger("LibraryFeature");
const QString kIconPath = QStringLiteral(":/images/library/ic_library_%1.svg");

} // anonymous namespace

LibraryFeature::LibraryFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig,
        const QString& iconName)
        : QObject(pLibrary),
          m_pLibrary(pLibrary),
          m_pConfig(pConfig),
          m_iconName(iconName) {
    if (!m_iconName.isEmpty()) {
        m_icon = QIcon(kIconPath.arg(m_iconName));
    }
}

void LibraryFeature::selectAndActivate(const QModelIndex& index) {
    if (index.isValid()) {
        emit featureSelect(this, index);
        activateChild(index);
    } else {
        // calling featureSelect with invalid index will select the root item
        emit featureSelect(this, QModelIndex());
        activate();
    }
}
