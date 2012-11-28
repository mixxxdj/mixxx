#include <QDebug>

#include "bpmeditor.h"

BPMEditor::BPMEditor(const QStyleOptionViewItem& option,
                    EditMode mode, QWidget *parent)
          :QWidget(parent),
           m_pLock(new BPMButton(this)),
           m_pBPM(new QDoubleSpinBox(this)),
           m_pLayout(new QHBoxLayout(this)),
           m_isSelected(option.state & QStyle::State_Selected) {
    setPalette(option.palette);
    // configure Lock Button
    m_pLock->setMaximumWidth(20);
    // configure SpinBox
    m_pBPM->setMinimum(0);
    m_pBPM->setMaximum(1000);
    m_pBPM->setSingleStep(0.1);
    if (mode == Editable) {
        m_pBPM->setDecimals(10);
        qDebug() << "is in editmode";
    }
    //configure Layout
    m_pLayout->addWidget(m_pLock);
    m_pLayout->addSpacing(2);
    m_pLayout->addWidget(m_pBPM);
    m_pLayout->setContentsMargins(0,0,0,0);
    m_pLayout->setSpacing(0);
    //add all to our widget
    setLayout(m_pLayout);
    //connect signals
    connect(m_pLock, SIGNAL(clicked(bool)), this, SIGNAL(finishedEditing()));
    connect(m_pBPM, SIGNAL(valueChanged(double)),
            this, SIGNAL(finishedEditing()));
}

BPMEditor::~BPMEditor(){
    delete m_pLock;
    delete m_pBPM;
    delete m_pLayout;
}

bool BPMEditor::getLock(){
    return m_pLock->isChecked();
}

double BPMEditor::getBPM(){
    return m_pBPM->value();
}

void BPMEditor::setData(const QModelIndex &index, int lockColumn){
    m_pBPM->setValue(index.data().toDouble());
    m_pLock->setChecked(index.sibling(index.row(),lockColumn).data().toBool());
}
