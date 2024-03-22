#include "library/tabledelegates/bpmdelegate.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QItemEditorCreatorBase>
#include <QItemEditorFactory>
#include <QPainter>
#include <QTableView>

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
        pBpmSpinbox->setMaximum(1000);
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
        : TableItemDelegate(pTableView),
          m_pCheckBox(new QCheckBox(m_pTableView)) {
    m_pCheckBox->setObjectName("LibraryBPMButton");
    // NOTE(rryan): Without ensurePolished the first render of the QTableView
    // shows the checkbox unstyled. Not sure why -- but this fixes it.
    m_pCheckBox->ensurePolished();
    m_pCheckBox->hide();

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

    // Actually QAbstractTableModel::data(index, BackgroundRole) provides the
    // correct custom background color (track color).
    // Though, since Qt6 the above style rules would not apply for some reason,
    // (see bug #11630) which can be fixed by also setting
    // #LibraryBPMButton::item { border: 0px;}
    // This however enables some default styles and clears the custom background
    // color (track color), see bug #12355 ¯\_(ツ)_/¯ Qt is fun!
    // Fix that by setting the bg color explicitly here.
    paintItemBackground(painter, option, index);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // The checkbox uses the QTableView's qss style, therefore it's not picking
    //  up the 'played' text color via ForegroundRole from BaseTrackTableModel::data().
    // Enforce it with an explicit stylesheet. Note: the stylesheet persists so
    // we need to reset it to normal/highlighted.
    QColor textColor;
    auto dat = index.data(Qt::ForegroundRole);
    if (dat.canConvert<QColor>()) {
        textColor = dat.value<QColor>();
    } else {
        if (option.state & QStyle::State_Selected) {
            textColor = option.palette.color(QPalette::Normal, QPalette::HighlightedText);
        } else {
            textColor = option.palette.color(QPalette::Normal, QPalette::Text);
            // qWarning() << "     !! BPM col:" << textColor.name(QColor::HexRgb);
        }
    }
    if (textColor.isValid()) {
        m_pCheckBox->setStyleSheet(QStringLiteral(
                "#LibraryBPMButton::item { color: %1; }")
                                           .arg(textColor.name(QColor::HexRgb)));
    }

    QStyle* style = m_pTableView->style();
    if (style != nullptr) {
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, m_pCheckBox);
    }
}
