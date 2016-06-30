#include <QStringBuilder>

#include <widget/wlibrarybreadcrumb.h>

namespace {

QString getPathString(TreeItem* pTree) {
    // Base case
    if (pTree->parent() == nullptr) {
        return pTree->getFeature()->title().toString();
    }
    
    // Recursive case
    QString text = pTree->data().toString();
    QString next = getPathString(pTree->parent());
    return next % QLatin1Literal(" > ") % text;
}

} // NAMESPACE


WLibraryBreadCrumb::WLibraryBreadCrumb(QWidget* parent) 
		: QLabel(parent) {
	setText("I'm a BreadCrumb");
}

void WLibraryBreadCrumb::showBreadCrumb(TreeItem *pTree) {
    QString text = getPathString(pTree);
    setText(text);
}
