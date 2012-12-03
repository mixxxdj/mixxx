#include "bpmbutton.h"

#include <QPainter>
#include <QImage>
#include <QRgb>
#include <QRect>

BPMButton::BPMButton(QWidget *parent) : QAbstractButton(parent),
                                        m_Checked("res/images/library/checked.png"){
    setCheckable(true);
    setChecked(false);
    //convert image to greyscale
    QImage image("res/images/library/checked.png");
    QRgb col;
    int gray;
    int width = m_Checked.width();
    int height = m_Checked.height();
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            col = image.pixel(i, j);
            gray = qGray(col);
            image.setPixel(i, j, qRgb(gray, gray, gray));
        }
    }
    m_Unchecked = m_Unchecked.fromImage(image);
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
