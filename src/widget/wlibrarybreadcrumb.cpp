#include <QStringBuilder>

#include "library/treeitemmodel.h"
#include <widget/wlibrarybreadcrumb.h>

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

void WLibraryBreadCrumb::showBreadCrumb(TreeItem* pTree) {
    setText(TreeItemModel::getBreadCrumbString(pTree));
}

void WLibraryBreadCrumb::showBreadCrumb(const QString& text) {
    setText(text);
}

void WLibraryBreadCrumb::resizeEvent(QResizeEvent *pEvent) {
    QLabel::resizeEvent(pEvent);
    setText(m_longText);
}
