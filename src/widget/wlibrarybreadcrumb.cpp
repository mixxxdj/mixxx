#include <QHBoxLayout>

#include "widget/wlibrarybreadcrumb.h"

#include "library/treeitemmodel.h"
#include "library/treeitem.h"

WLibraryBreadCrumb::WLibraryBreadCrumb(QWidget* parent) 
		: QWidget(parent) {
    QHBoxLayout* layout = new QHBoxLayout(this);
    m_pIcon = new QLabel(this);
    m_pText = new QLabel(this);
    layout->addWidget(m_pIcon);
    layout->addWidget(m_pText);
    layout->addItem(new QSpacerItem(0,0, QSizePolicy::MinimumExpanding));
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    setContentsMargins(0,0,0,0);
    setLayout(layout);
}

QSize WLibraryBreadCrumb::minimumSizeHint() const {
    return QSize(0, height());
}

void WLibraryBreadCrumb::showBreadCrumb(TreeItem* pTree) {
    LibraryFeature* pFeature = pTree->getFeature();
    DEBUG_ASSERT_AND_HANDLE(pFeature) {
        return;
    }
    
    setBreadIcon(pFeature->getIcon());
    setText(TreeItemModel::getBreadCrumbString(pTree));
}

void WLibraryBreadCrumb::showBreadCrumb(const QString& text, const QIcon& icon) {
    setText(text);
    setBreadIcon(icon);
}

void WLibraryBreadCrumb::setBreadIcon(const QIcon& icon) {
    // Get font height
    int height = fontMetrics().height();    
    m_pIcon->setPixmap(icon.pixmap(height));
}

void WLibraryBreadCrumb::resizeEvent(QResizeEvent* pEvent) {
    QWidget::resizeEvent(pEvent);
    refreshWidth();
}

void WLibraryBreadCrumb::setText(const QString &text) {
    m_longText = text;
    refreshWidth();
}

void WLibraryBreadCrumb::refreshWidth() {
    QFontMetrics metrics(fontMetrics());
    
    // Measure the text for the label width
    int mLText, mRText;
    m_pText->getContentsMargins(&mLText, nullptr, &mRText, nullptr);
    int margins = mLText + mRText;
    
    int newSize = width() - m_pIcon->width() - margins;
    QString elidedText = metrics.elidedText(m_longText, Qt::ElideRight, newSize);
    m_pText->setText(elidedText);
}
