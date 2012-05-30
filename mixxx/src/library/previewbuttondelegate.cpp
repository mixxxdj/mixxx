#include <QPainter>
#include <QPushButton>

#include "previewbuttondelegate.h"
#include "library/previewdeckbuttonhandler.h"

PreviewButtonDelegate::PreviewButtonDelegate(QObject *parent, int column) :
    QStyledItemDelegate(parent)
{
    if(QTableView *tableView = qobject_cast<QTableView *>(parent))
    {
        m_pTableView = tableView;
        m_pButton = new QPushButton("", m_pTableView);
        m_pButton->setIcon(QIcon("res/btn_play_sampler.png"));
        m_pButton->hide();
        // m_pTableView->setMouseTracking(true);
        connect(m_pTableView, SIGNAL(entered(QModelIndex)),
                this, SLOT(cellEntered(QModelIndex)));

        //TODO(kain88) right now this assumes the parent is wtracktableview
        //and therefore this works, it should check before the connection 
        //is made and raise an error if not
        connect(this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)), 
                parent, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));

        m_isOneCellInEditMode = false;
        m_column=column;
    }
}

PreviewButtonDelegate::~PreviewButtonDelegate()
{
    delete m_pButton;
}

QWidget * PreviewButtonDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    Q_UNUSED(option);
    //TODO(kain88) Memory leak or does Qt take care about that for us?
    QPushButton * btn = new QPushButton(parent);
    btn->setIcon(QIcon("res/btn_play_sampler.png"));
    //the handle will emit the signal to load the track
    PreviewdeckButtonHandler *phandle = new PreviewdeckButtonHandler(this,
                                                    index, m_pTableView);
    connect(btn,SIGNAL(clicked()),phandle,SLOT(buttonclicked()));

    return btn;
}

void PreviewButtonDelegate::setEditorData(QWidget *editor,
                                 const QModelIndex &index) const
{
    Q_UNUSED(editor);
    Q_UNUSED(index);
}

void PreviewButtonDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const
{
    Q_UNUSED(editor);
    Q_UNUSED(model);
    Q_UNUSED(index);
}

void PreviewButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    m_pButton->setGeometry(option.rect);
    if (option.state == QStyle::State_Selected)
                    painter->fillRect(option.rect, option.palette.base());
    QPixmap map = QPixmap::grabWidget(m_pButton);
    painter->drawPixmap(option.rect.x(),option.rect.y(),map);
}

void PreviewButtonDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

QSize PreviewButtonDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return m_pButton->sizeHint();
}

void PreviewButtonDelegate::cellEntered(const QModelIndex &index)
{
    //this slot is called if the mouse pointer enters ANY cell on
    //the QTableView but the code should only be executed on a button
    if(index.column()==m_column)
    {
        if(m_isOneCellInEditMode)
        {
            m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
        }
        m_pTableView->openPersistentEditor(index);
        m_isOneCellInEditMode = true;
        m_currentEditedCellIndex = index;
    } else {
        if(m_isOneCellInEditMode)
        {
            m_isOneCellInEditMode = false;
            m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
        }
    }
}
