#include "wlibrary.h"

#include "moc_wlibrary.cpp"

const QString parentName = QStringLiteral("WLibrary");

const bool sDebug = false;

WLibrary::WLibrary(QWidget* parent)
        : WLibraryBaseWindow(parent) {
    const_cast<QString&>(m_callingParent) = "WLibrary"; // Override parentName
    if (sDebug) {
        qDebug() << "WLibraryPreparationWindow initialized with parent:" << m_callingParent;
        qDebug() << "WLibrary initialized as WLibraryBaseWindow";
    }
}
