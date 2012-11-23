#include <QPainter>
#include <QFont>
#include <QDebug>

#include "bpmdelegate.h"

BPMDelegate::BPMDelegate(QObject *parent, int column)
                     : QStyledItemDelegate(parent) {
    if (QTableView *tableView = qobject_cast<QTableView *>(parent)) {
        m_pTableView = tableView;

        m_pWidget = new QWidget;

        m_pBPM = new QLabel;
        m_pBPM->setText("349");
        QFont f("Arial", 7, QFont::Normal);
        m_pBPM->setFont(f);
        m_pBPM->setMinimumWidth(1);
        m_pButton = new QPushButton("", m_pTableView);
        //TODO(kain88) the place of res/ is saved in the config, no obvious easy
        // way to get this here
        m_pButton->setIcon(QIcon("res/images/library/Play.png"));
        m_pButton->setMinimumWidth(1);
        // m_pButton->hide();

        m_pLayout = new QHBoxLayout;
        m_pLayout->addWidget(m_pButton);
        m_pLayout->addWidget(m_pBPM);

        m_pWidget->setLayout(m_pLayout);

        connect(m_pTableView, SIGNAL(entered(QModelIndex)),
                this, SLOT(cellEntered(QModelIndex)));
        m_isOneCellInEditMode = false;
        m_column=column;
    }
}

BPMDelegate::~BPMDelegate() {
}

QWidget * BPMDelegate::createEditor(QWidget *parent,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    QPushButton* btn = new QPushButton(parent);
    btn->setIcon(QIcon("res/images/library/Play.png"));
    return btn;
}

void BPMDelegate::setEditorData(QWidget *editor,
                                          const QModelIndex &index) const {
    Q_UNUSED(editor);
    Q_UNUSED(index);
}

void BPMDelegate::setModelData(QWidget *editor,
                                         QAbstractItemModel *model,
                                         const QModelIndex &index) const {
    Q_UNUSED(editor);
    Q_UNUSED(model);
    Q_UNUSED(index);
}

void BPMDelegate::paint(QPainter *painter,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const {
    Q_UNUSED(index);
    qDebug() << option.rect.x() << '\t' << option.rect.y();
    m_pButton->setGeometry(option.rect);
    m_pBPM->setGeometry(option.rect);
    m_pWidget->setGeometry(option.rect);
    if (option.state == QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.base());
    QPixmap map = QPixmap::grabWidget(m_pWidget);
    painter->drawPixmap(option.rect.x(),option.rect.y(),map);
}

void BPMDelegate::updateEditorGeometry(QWidget *editor,
                                                 const QStyleOptionViewItem &option,
                                                 const QModelIndex &index) const {
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

QSize BPMDelegate::sizeHint(const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    return m_pWidget->sizeHint();
}

void BPMDelegate::cellEntered(const QModelIndex &index) {
    //this slot is called if the mouse pointer enters ANY cell on
    //the QTableView but the code should only be executed on a button
    if (index.column()==m_column) {
        if (m_isOneCellInEditMode) {
            m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
        }
        m_pTableView->openPersistentEditor(index);
        m_isOneCellInEditMode = true;
        m_currentEditedCellIndex = index;
    } else { // close editor if the mouse leaves the button
        if (m_isOneCellInEditMode) {
            m_isOneCellInEditMode = false;
            m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
        }
    }
}
