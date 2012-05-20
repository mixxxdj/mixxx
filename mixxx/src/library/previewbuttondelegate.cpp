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
#include "controlobjectthreadmain.h"
#include "controlobject.h"

PreviewButtonDelegate::PreviewButtonDelegate(QObject *parent, int column) :
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
        
        connect(this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)), parent, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));
        m_isOneCellInEditMode = false;
        m_group = QString("[PreviewDeck1]");//currently there is only one previewDeck
        m_column=column;
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
    /*
    if(index.column()!=m_column){
        qDebug() << "kain88 create the editor on normal cell";
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
    */
    //Memory leak?
    QPushButton * btn = new QPushButton(parent);
    btn->setText(qVariantValue<QString>(index.data()));
    // qDebug() <<"kain88 index.data() = "<<index.data();
    
    connect(btn,SIGNAL(clicked()),this,SLOT(buttonclicked()));

    return btn;
}

//setEditorData
void PreviewButtonDelegate::setEditorData(QWidget *editor,
                                 const QModelIndex &index) const
{
    // if (index.column()==m_column) {
        QPushButton * btn = qobject_cast<QPushButton *>(editor);
        btn->setProperty("data_value", index.data());
    /*
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
    */
}

//setModelData
void PreviewButtonDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const
{
    // if (index.column()==m_column) {
        QPushButton *btn = qobject_cast<QPushButton *>(editor);
        model->setData(index, btn->property("data_value"));
    /*
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
    */
}
//paint
void PreviewButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    /*
    if (index.column()!=m_column) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    */
    
    m_pBtn->setGeometry(option.rect);
    m_pBtn->setText(qVariantValue<QString>(index.data()));
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
    TrackModel* ptrackModel = dynamic_cast<TrackModel*>(m_pMyWidget->model());
    QModelIndexList indexes = m_pMyWidget->selectionModel()->selectedIndexes();
    //if no row is selected then indexes is empty and Qt crashes
    //ok this is rather strange,if a row is selected at the beginning everything is
    // fine until i play a song, then i have to select a new row to get it working again
    
    //check if deck is playing and stop it
    ControlObjectThreadMain* playStatus = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(m_group, "play")));
    if(!playStatus->get()){
        playStatus->slotSet(0);
    } 
    if(!indexes.empty()){
        TrackPointer Track = ptrackModel->getTrack(indexes.at(0));
        qDebug() << "kain88 Track="<<Track;
        // qDebug() <<index.row();
        emit(loadTrackToPlayer(Track,m_group));
    }
}
