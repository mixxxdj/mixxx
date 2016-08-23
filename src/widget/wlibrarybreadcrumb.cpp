#include <QHBoxLayout>

#include "widget/wlibrarybreadcrumb.h"

#include "library/treeitemmodel.h"

WLibraryBreadCrumb::WLibraryBreadCrumb(QWidget* parent) 
		: QWidget(parent) {
    QHBoxLayout* layout = new QHBoxLayout(this);
    m_pIcon = new QLabel(this);    
    m_pText = new QLabel(this);
    layout->addWidget(m_pIcon);
    layout->addWidget(m_pText);
    layout->addItem(new QSpacerItem(0,0, QSizePolicy::MinimumExpanding));
    layout->setSpacing(2);
    layout->setContentsMargins(0,0,0,0);
    layout->setAlignment(Qt::AlignVCenter);
    
    setContentsMargins(0,0,0,0);
    setLayout(layout);
}

// Do not remove this, if the minimum size hint is not 0 width then when
// the user resizes a panel with the breadcrumb it never shows elided text and
// the minimum size becomes the text lenght
QSize WLibraryBreadCrumb::minimumSizeHint() const {
    QSize min = m_pText->minimumSizeHint();
    return QSize(0, min.height());
}

void WLibraryBreadCrumb::showBreadCrumb(TreeItem* pTree) {
    LibraryFeature* pFeature = pTree->getFeature();
    DEBUG_ASSERT_AND_HANDLE(pFeature != nullptr) {
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
    int height = m_pText->fontMetrics().height();
    QPixmap pix = icon.pixmap(height);
    m_pIcon->setContentsMargins(0, 0, 0, 0);
    m_pIcon->setPixmap(pix);
}

void WLibraryBreadCrumb::resizeEvent(QResizeEvent* pEvent) {
    QWidget::resizeEvent(pEvent);
    refreshWidth();
}

void WLibraryBreadCrumb::setText(const QString &text) {
    m_longText = text;
    setToolTip(m_longText);
    refreshWidth();
}

void WLibraryBreadCrumb::refreshWidth() {
    QFontMetrics metrics(fontMetrics());
    
    // Measure the text for the label width
    int mLText, mRText, mLIcon, mRIcon;
    m_pText->getContentsMargins(&mLText, nullptr, &mRText, nullptr);
    m_pIcon->getContentsMargins(&mLIcon, nullptr, &mRIcon, nullptr);
    int margins = mLText + mRText + layout()->spacing() + mLIcon + mRIcon;
    
    int newSize = width() - m_pIcon->width() - margins;
    QString elidedText = metrics.elidedText(m_longText, Qt::ElideRight, newSize);
    m_pText->setText(elidedText);
}
