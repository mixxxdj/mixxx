#include <QHBoxLayout>

#include "widget/wlibrarybreadcrumb.h"

#include "library/treeitemmodel.h"
#include "widget/wpixmapstore.h"
#include "widget/wtristatebutton.h"

WLibraryBreadCrumb::WLibraryBreadCrumb(QWidget* parent) 
		: QWidget(parent),
          m_pIcon(new QLabel(this)),
          m_pText(new QLabel(this)),
          m_pPreselectButton(new WTriStateButton(this)),
          m_preselected(false) {
    QLayout* layout = new QHBoxLayout(this);
    
    m_pIcon->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_pIcon);
    layout->addWidget(m_pText);
    
    QPixmap preHovered(WPixmapStore::getLibraryPixmap(
                        ":/images/library/ic_library_preselect.png"));
    QPixmap preOff(WPixmapStore::getLibraryPixmap(
                        ":/images/library/ic_library_notpreselect.png"));
    QPixmap preLock(WPixmapStore::getLibraryPixmap(
                        ":/images/library/ic_library_lockpreselect.png"));
    
    m_preselectIcon.addPixmap(preLock, QIcon::Normal, QIcon::On);
    m_preselectIcon.addPixmap(preOff, QIcon::Normal, QIcon::Off);
    m_preselectIcon.addPixmap(preHovered, QIcon::Active, QIcon::Off);
    m_pPreselectButton->setIcon(m_preselectIcon);
    m_pPreselectButton->setChecked(m_preselected);
    m_pPreselectButton->setFocusPolicy(Qt::ClickFocus);
    
    connect(m_pPreselectButton, SIGNAL(clicked(bool)), 
            this, SIGNAL(preselected(bool)));
    
    layout->addItem(new QSpacerItem(0,0, QSizePolicy::MinimumExpanding));
    layout->addWidget(m_pPreselectButton);
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

void WLibraryBreadCrumb::setPreselected(bool value) {
    m_preselected = value;
    m_pPreselectButton->setChecked(value);
    refreshWidth();
}

bool WLibraryBreadCrumb::isPreselected() const {
    return m_preselected;
}

void WLibraryBreadCrumb::setPreviewed(bool value) {
    m_pPreselectButton->setHovered(value);
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
    int mLText, mRText, mLIcon, mRIcon, mLPLabel, mRPLabel;
    m_pText->getContentsMargins(&mLText, nullptr, &mRText, nullptr);
    m_pIcon->getContentsMargins(&mLIcon, nullptr, &mRIcon, nullptr);
    m_pPreselectButton->getContentsMargins(&mLPLabel, nullptr, &mRPLabel, nullptr);
    int margins = mLIcon + mRIcon + layout()->spacing() + 
            mLText + mRText + mLPLabel + mRPLabel;
    
    int newSize = width() - m_pIcon->width() - margins - 
            m_pPreselectButton->width();
    QString elidedText = metrics.elidedText(m_longText, Qt::ElideRight, newSize);
    m_pText->setText(elidedText);
}
