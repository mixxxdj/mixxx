#include "library/tabledelegates/bpmdelegate.h"

#include <QDoubleSpinBox>
#include <QItemEditorCreatorBase>
#include <QItemEditorFactory>

#include "moc_bpmdelegate.cpp"

// We override the typical QDoubleSpinBox editor by registering this class with
// a QItemEditorFactory for the BPMDelegate.
class BpmEditorCreator : public QItemEditorCreatorBase {
  public:
    BpmEditorCreator() {}
    ~BpmEditorCreator() override {
    }

    QWidget* createWidget(QWidget* parent) const override {
        QDoubleSpinBox* pBpmSpinbox = new QDoubleSpinBox(parent);
        pBpmSpinbox->setFrame(false);
        pBpmSpinbox->setMinimum(0);
        pBpmSpinbox->setMaximum(9999);
        pBpmSpinbox->setSingleStep(1e-3);
        pBpmSpinbox->setDecimals(8);
        pBpmSpinbox->setObjectName("LibraryBPMSpinBox");
        return pBpmSpinbox;
    }

    QByteArray valuePropertyName() const override {
        return QByteArray("value");
    }
};

BPMDelegate::BPMDelegate(QTableView* pTableView)
        : CheckboxDelegate(pTableView, QStringLiteral("LibraryBPMButton")) {
    // Register a custom QItemEditorFactory to override the default
    // QDoubleSpinBox editor.
    m_pFactory = new QItemEditorFactory();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_pFactory->registerEditor(QMetaType::Double, new BpmEditorCreator());
#else
    m_pFactory->registerEditor(QVariant::Double, new BpmEditorCreator());
#endif
    setItemEditorFactory(m_pFactory);
}

void BPMDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    const QColor highlightBg = paintHighlightOverSelection(painter, option, index);
    if (!highlightBg.isValid() || !(option.state & QStyle::State_Selected)) {
        CheckboxDelegate::paintItem(painter, option, index);
        return;
    }
    // The BPM highlighter tinted this selected cell. The stock skins give
    // #LibraryBPMButton::item:selected an opaque background, which the style
    // sheet style paints regardless of the palette, so draw the item as if it
    // were unselected: the skins' plain ::item rule has no background, so the
    // tint stays visible, and the text uses palette Text, i.e. the model's
    // contrasting ForegroundRole colour. Clear the BackgroundRole brush too, so
    // a skin without an ::item rule doesn't paint the tint opaquely over the
    // selection. `option` already went through initStyleOption() in
    // TableItemDelegate::paint().
    QStyleOptionViewItem opt = option;
    opt.state &= ~QStyle::State_Selected;
    opt.backgroundBrush = QBrush();
    drawCheckboxItem(painter, opt);
}
