#include <QStringBuilder>

#include <widget/wlibrarybreadcrumb.h>

namespace {

QString getPathString(TreeItem* pTree) {    
    // Base case
    if (pTree == nullptr || pTree->getFeature() == nullptr) {
        return QString();
    }
    else if (pTree->parent() == nullptr) {
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

void WLibraryBreadCrumb::setText(const QString &text) {
    m_longText = text;
    QFontMetrics metrics(font());
    
    // Measure the text for the label width
    QString elidedText = metrics.elidedText(m_longText, Qt::ElideRight, width() - 10);
    QLabel::setText(elidedText);
}

QString WLibraryBreadCrumb::text() const {
    return m_longText;
}

QSize WLibraryBreadCrumb::minimumSizeHint() const {
    return QSize(0, height());
}

void WLibraryBreadCrumb::showBreadCrumb(TreeItem *pTree) {
    setText(getPathString(pTree));
}

void WLibraryBreadCrumb::resizeEvent(QResizeEvent *pEvent) {
    QLabel::resizeEvent(pEvent);
    setText(m_longText);
}
