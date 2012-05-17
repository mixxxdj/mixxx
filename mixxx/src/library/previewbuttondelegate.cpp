#include <QPainter>
#include <QPushButton>
#include <QStylePainter>
#include <QDebug>
#include <QTableView>

#include "previewbuttondelegate.h"
#include "starrating.h"
#include "trackinfoobject.h"
#include "library/trackmodel.h"
#include "library/librarytablemodel.h"//Do I really need this???

PreviewButtonDelegate::PreviewButtonDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
    if(QTableView *tableView = qobject_cast<QTableView *>(parent))
    {
        m_pMyWidget = tableView;
        m_pBtn = new QPushButton("...", m_pMyWidget);
        m_pBtn->hide();
        m_pMyWidget->setMouseTracking(true);
        connect(m_pMyWidget, SIGNAL(entered(QModelIndex)),
                              this, SLOT(cellEntered(QModelIndex)));
         /*
        connect(this,SIGNAL(loadTrackToPlayer(pTrack, group)),
         which object?,SIGNAL(loadTrackToPlayer(pTrack,deckGroup)));
         */
        m_isOneCellInEditMode = false;
        m_group = QString("[PreviewDeck1]");//currently there is only one previewDeck
        m_column=3;
    }
}

PreviewButtonDelegate::~PreviewButtonDelegate()
{
    delete m_pBtn;
}
//createEditor
QWidget * PreviewButtonDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    if(index.column()!=m_column){
        qDebug() << "kain88 create the editor on normal cell";
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
    QPushButton * btn = new QPushButton(parent);
    btn->setText("play");
    qDebug() <<"kain88 index.data() = "<<index.data();
    
    //TODO(kain88) memory leak?
    // TrackModel* trackModel = dynamic_cast<TrackModel*>(model());
    // m_pTrack = trackModel->getTrack(index);
    
    //m_index=index;can't do that since there is no valid copy constructor
    connect(btn,SIGNAL(clicked()),this,SLOT(buttonclicked()));

    return btn;
}

//setEditorData
void PreviewButtonDelegate::setEditorData(QWidget *editor,
                                 const QModelIndex &index) const
{
    if (qVariantCanConvert<StarRating>(index.data())) {
        QPushButton * btn = qobject_cast<QPushButton *>(editor);
        btn->setProperty("data_value", index.data());
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

//setModelData
void PreviewButtonDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const
{
    if (qVariantCanConvert<StarRating>(index.data())) {
        QPushButton *btn = qobject_cast<QPushButton *>(editor);
        model->setData(index, btn->property("data_value"));
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}
//paint
void PreviewButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column()!=m_column) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    
    m_pBtn->setGeometry(option.rect);
    m_pBtn->setText("play");
    if (option.state == QStyle::State_Selected)
                    painter->fillRect(option.rect, option.palette.highlight());
    QPixmap map = QPixmap::grabWidget(m_pBtn);
    painter->drawPixmap(option.rect.x(),option.rect.y(),map);
}
//updateGeometry
void PreviewButtonDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

QSize PreviewButtonDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const{
    return QStyledItemDelegate::sizeHint(option, index);
}

//cellEntered
void PreviewButtonDelegate::cellEntered(const QModelIndex &index)
{

    if(index.column()==m_column)
    {
        // qDebug() << "kain88 cell entered";
        if(m_isOneCellInEditMode)
        {
            m_pMyWidget->closePersistentEditor(m_currentEditedCellIndex);
        }
        m_pMyWidget->openPersistentEditor(index);
        m_isOneCellInEditMode = true;
        m_currentEditedCellIndex = index;
    } else {
        if(m_isOneCellInEditMode)
        {
            m_isOneCellInEditMode = false;
            m_pMyWidget->closePersistentEditor(m_currentEditedCellIndex);
        }
    }
}

void PreviewButtonDelegate::buttonclicked(){
    qDebug() << "kain88 button clicked";
    // emit(loadTrackToPlayer(m_pTrack,m_group))
}
