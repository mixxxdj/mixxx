#include "bpmbutton.h"

#include <QPainter>

BPMButton::BPMButton(QWidget *parent) : QAbstractButton(parent),
                                        m_Checked("res/images/library/checked.png"),
                                        m_Unchecked("res/images/library/unchecked.png") {
    setCheckable(true);
    setChecked(false);
}

BPMButton::~BPMButton(){
}

void BPMButton::setCheckedImage(QPixmap &image){
    m_Checked = image;
}

void BPMButton::setUncheckedImage(QPixmap &image){
    m_Unchecked = image;
}

void BPMButton::paintEvent(QPaintEvent *e){
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setBrush(Qt::blue);
    painter.setPen(Qt::NoPen);
    if (isChecked())
        painter.drawPixmap(this->x(),this->y(),m_Checked);
    else
        painter.drawPixmap(this->x(),this->y(),m_Unchecked);
}
