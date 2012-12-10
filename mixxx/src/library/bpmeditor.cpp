#include "bpmeditor.h"

BPMEditor::BPMEditor(EditMode mode, QWidget *parent)
          :QWidget(parent),
           m_pLock(new QPushButton(this)),
           m_pBPMSpinBox(NULL),
           m_pBPMLabel(NULL),
           m_pLayout(new QHBoxLayout(this)) {
    setObjectName("LibraryBPM");
    // configure Lock Button
    m_pLock->setMaximumWidth(20);
    m_pLock->setCheckable(true);
    m_pLock->setChecked(false);
    m_pLock->setObjectName("LibraryBPMButton");
    m_pLayout->addWidget(m_pLock);
    // configure SpinBox of Label depending on mode
    if (mode==Editable && !getLock()) {
        m_pBPMSpinBox = new QDoubleSpinBox(this);
        m_pBPMSpinBox->setMinimum(0);
        m_pBPMSpinBox->setMaximum(1000);
        m_pBPMSpinBox->setSingleStep(1e-8);
        m_pBPMSpinBox->setDecimals(8);
        m_pBPMSpinBox->setObjectName("LibraryBPMSpinBox");
        m_pLayout->addWidget(m_pBPMSpinBox);
        connect(m_pBPMSpinBox, SIGNAL(editingFinished()),
                this, SIGNAL(finishedEditing()));
    } else {
        m_pBPMLabel = new QLabel(this);
        m_pLayout->addWidget(m_pBPMLabel);
    }
    //configure Layout
    m_pLayout->setContentsMargins(0,0,0,0);
    m_pLayout->setSpacing(0);
    //add all to our widget
    setLayout(m_pLayout);
    //connect signals
    connect(m_pLock, SIGNAL(clicked(bool)), this, SIGNAL(finishedEditing()));
}

BPMEditor::~BPMEditor(){
    delete m_pLock;
    delete m_pBPMSpinBox;
    delete m_pLayout;
}

bool BPMEditor::getLock(){
    return m_pLock->isChecked();
}

double BPMEditor::getBPM(){
    return m_pBPMSpinBox->value();
}

void BPMEditor::setData(const QModelIndex &index, int lockColumn){
    if (m_pBPMSpinBox) {
        m_pBPMSpinBox->setValue(index.data().toDouble());
    } else {
        // cut of BPM two digits after the dot
        int bpm = index.data().toDouble()*1e2;
        m_pBPMLabel->setText(QString::number(bpm*1e-2));
    }
    m_pLock->setChecked(index.sibling(index.row(),lockColumn).data().toBool());
}
