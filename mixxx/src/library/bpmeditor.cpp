#include "bpmeditor.h"

BPMEditor::BPMEditor(const QStyleOptionViewItem& option, QWidget *parent)
          :QWidget(parent),
           m_pLock(new BPMButton(this)),
           m_pBPM(new QDoubleSpinBox(this)),
           m_isSelected(option.state & QStyle::State_Selected),
           m_pLayout(new QHBoxLayout(this)){
    setPalette(option.palette);
    // configure Lock Button
    m_pLock->setMaximumWidth(20);
    // configure SpinBox
    m_pBPM->setMinimum(0);
    m_pBPM->setMaximum(1000);
    m_pBPM->setSingleStep(0.1);
    //configure Layout
    m_pLayout->addWidget(m_pLock);
    // m_pLayout->addSpacing();
    m_pLayout->setContentsMargins(0,0,0,0);
    m_pLayout->setSpacing(0);
    //add all to our widget
    setLayout(m_pLayout);
}

BPMEditor::~BPMEditor(){
    delete m_pLock;
    delete m_pBPM;
    delete m_pLayout;
}

void BPMEditor::paintEvent(QPaintEvent *event, const QStyleOptionViewItem &option,
                           const QModelindex &index) {
    
}

void BPMEditor::paint(QPainter *painter, const QRect &rect,
                      const QPalette &palette, EditMode mode,
                      bool isSelected, int lockColumn,
                      const QModelIndex &index) const {
    painter->save();

    setGeometry(option.rect);
    m_pBPM->setValue(index.data().toDouble());
    m_pLock->setChecked(index.sibling(index.row(),lockColumn).data().toBool());
    if (option.state == QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.base());
    QPixmap map = QPixmap::grabWidget(this);
    painter->drawPixmap(option.rect.x(),option.rect.y(),map);
}

void BPMEditor::setEditorData(const QModelIndex &index, int lockColumn)
