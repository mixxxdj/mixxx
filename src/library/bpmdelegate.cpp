#include <QItemEditorFactory>
#include <QItemEditorCreatorBase>
#include <QDoubleSpinBox>
#include <QRect>
#include <QPalette>
#include <QTableView>
#include <QPainter>

#include "library/bpmdelegate.h"
#include "library/trackmodel.h"

// We override the typical QDoubleSpinBox editor by registering this class with
// a QItemEditorFactory for the BPMDelegate.
class BpmEditorCreator : public QItemEditorCreatorBase {
  public:
    BpmEditorCreator() {}
    virtual ~BpmEditorCreator() {}

    virtual QWidget* createWidget (QWidget* parent) const {
        QDoubleSpinBox* pBpmSpinbox = new QDoubleSpinBox(parent);
        pBpmSpinbox->setFrame(false);
        pBpmSpinbox->setMinimum(0);
        pBpmSpinbox->setMaximum(1000);
        pBpmSpinbox->setSingleStep(1e-3);
        pBpmSpinbox->setDecimals(8);
        pBpmSpinbox->setObjectName("LibraryBPMSpinBox");
        return pBpmSpinbox;
    }

    virtual QByteArray valuePropertyName() const {
        return QByteArray("value");
    }
};

BPMDelegate::BPMDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView),
          m_pTableView(pTableView),
          m_pCheckBox(new QCheckBox(m_pTableView)) {
    m_pCheckBox->setObjectName("LibraryBPMButton");
    // NOTE(rryan): Without ensurePolished the first render of the QTableView
    // shows the checkbox unstyled. Not sure why -- but this fixes it.
    m_pCheckBox->ensurePolished();
    m_pCheckBox->hide();

    // Register a custom QItemEditorFactory to override the default
    // QDoubleSpinBox editor.
    m_pFactory = new QItemEditorFactory();
    m_pFactory->registerEditor(QVariant::Double, new BpmEditorCreator());
    setItemEditorFactory(m_pFactory);
}

BPMDelegate::~BPMDelegate() {
    delete m_pFactory;
}

void BPMDelegate::paintItem(QPainter* painter,const QStyleOptionViewItem &option,
                        const QModelIndex& index) const {
    // NOTE(rryan): Qt has a built-in limitation that we cannot style multiple
    // CheckState indicators in the same QAbstractItemView. The CSS rule
    // QTableView::indicator:checked applies to all columns with a
    // CheckState. This is a big pain if we want to use CheckState roles on two
    // columns (i.e. the played column and the BPM column) with different
    // styling. We typically want a lock icon for the BPM check-state and a
    // check-box for the times-played column and may want more in the future.
    //
    // This workaround creates a hidden QComboBox named LibraryBPMButton. We use
    // the parent QTableView's QStyle with the hidden QComboBox as the source of
    // style rules to draw a CE_ItemViewItem.
    //
    // Here's how you would typically style the LibraryBPMButton:
    // #LibraryBPMButton::indicator:checked {
    //   image: url(:/images/library/ic_library_locked.svg);
    // }
    // #LibraryBPMButton::indicator:unchecked {
    //  image: url(:/images/library/ic_library_unlocked.svg);
    // }
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    if (m_pTableView != NULL) {
        QStyle* style = m_pTableView->style();
        if (style != NULL) {
            style->drawControl(QStyle::CE_ItemViewItem, &opt, painter,
                               m_pCheckBox);
        }
    }
}
