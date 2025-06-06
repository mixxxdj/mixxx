#include "wlibrarypreparationwindow.h"

#include "moc_wlibrarypreparationwindow.cpp"

const QString parentName = QStringLiteral("WLibraryPreparationWindow");

WLibraryPreparationWindow::WLibraryPreparationWindow(QWidget* parent)
        : WLibraryBaseWindow(parent) {
    const_cast<QString&>(m_callingParent) = "WLibraryPreparationWindow"; // Override parentName
    qDebug() << "WLibraryPreparationWindow initialized with parent:" << m_callingParent;

    qDebug() << "WLibraryPreparationWindow initialized as WLibraryBaseWindow";
}
