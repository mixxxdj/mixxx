#include "bpmbutton.h"

#include <QPainter>
#include <QRect>

BPMButton::BPMButton(QWidget *parent) : QAbstractButton(parent),
                                        m_Checked("res/images/library/checked.png"),
                                        m_Unchecked("res/images/library/unchecked.png") {
    setCheckable(true);
    setChecked(false);
}

BPMButton::~BPMButton(){
}

QSize BPMButton::sizeHint() const {
    if (isChecked())
        return m_Checked.size();
    else
        return m_Unchecked.size();
}

void BPMButton::paintEvent(QPaintEvent *e){
    Q_UNUSED(e);
    QPainter painter(this);
    if (isChecked())
        painter.drawPixmap(0,0,m_Checked);
    else
        painter.drawPixmap(0,0,m_Unchecked);
}
