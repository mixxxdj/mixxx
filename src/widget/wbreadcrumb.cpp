#include <QStringBuilder>

#include <wbreadcrumb.h>

namespace {

QString& getPathString(TreeItem* pTree) {
    // Base case
    if (pTree == nullptr) {
        return QString();
    }
    
    // Recursive case
    QString text = pTree->data().toString();
    QString& next = getData(pTree->parent());
    return (next.isEmpty() ? text : next % QLatin1Literal(" > ") % text);
}

}


WBreadCrumb::WBreadCrumb(QWidget* parent) 
		: QLabel(parent) {
	
}

void WBreadCrumb::setBreadText(TreeItem *pTree) {
    QString text = getData(pTree);
    setText(text);
}
