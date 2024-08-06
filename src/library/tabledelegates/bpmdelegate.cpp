#include "library/tabledelegates/bpmdelegate.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QItemEditorCreatorBase>
#include <QItemEditorFactory>
#include <QPainter>
#include <QPixmapCache>
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

void BPMDelegate::paintItem(QPainter* painter,
        const QStyleOptionViewItem& option,
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

    QColor textColor;
    if (option.state & QStyle::State_Selected) {
        textColor = option.palette.color(QPalette::Normal, QPalette::HighlightedText);
    } else {
        auto colorData = index.data(Qt::ForegroundRole);
        if (colorData.canConvert<QColor>()) {
            textColor = colorData.value<QColor>();
        } else {
            textColor = option.palette.color(QPalette::Normal, QPalette::Text);
        }
    }

    // Retrieve the check state and BPM value
    bool isChecked = index.data(Qt::CheckStateRole).toInt() == Qt::Checked;
    QString bpmValue =
            index.data(Qt::UserRole + 2)
                    .toString(); // BPM value is stored with UserRole + 2

    // Generate a unique cache key that now includes isChecked and bpmValue
    QString cacheKey = QString("BPMDelegate_%1_%2_%3_%4_%5")
                               .arg(option.rect.width())
                               .arg(option.rect.height())
                               .arg(textColor.name(QColor::HexRgb),
                                       isChecked ? "checked" : "unchecked")
                               .arg(bpmValue);

    QPixmap pixmap;
    if (!QPixmapCache::find(cacheKey, &pixmap)) {
        // Pixmap is not in the cache, so we create a new one
        pixmap = QPixmap(option.rect.size());
        pixmap.fill(Qt::transparent); // Fill uninitialized pixmap's background transparent
        QPainter pixmapPainter(&pixmap);

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        // The checkbox uses the QTableView's qss style, therefore it's not picking
        // up the 'missing' or 'played' text color via ForegroundRole from
        // BaseTrackTableModel::data().
        // Enforce it with an explicit stylesheet. Note: the stylesheet persists so
        // we need to reset it to normal/highlighted.

        m_pCheckBox->setStyleSheet(QStringLiteral(
                "#LibraryBPMButton::item { color: %1; }")
                                           .arg(textColor.name(QColor::HexRgb)));

        // Set the checkbox's origin to top-left and adjust the size to fit into the pixmap.
        opt.rect = QRect(QPoint(0, 0), option.rect.size());

        // Use the QTableView's style to draw a plain default checkbox onto the pixmap.
        QStyle* style = m_pTableView->style();
        if (style != nullptr) {
            style->drawControl(QStyle::CE_ItemViewItem, &opt, &pixmapPainter, m_pCheckBox);
        }

        pixmapPainter.end(); // Finish painting to the pixmap.

        // Save the pixmap in the cache
        QPixmapCache::insert(cacheKey, pixmap);
    }

    // Now draw the cached pixmap onto the original painter.
    painter->drawPixmap(option.rect.topLeft(), pixmap);
}
