
#include <skin/launchimage.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QStyleOption>
#include <QPainter>


LaunchImage::LaunchImage(QWidget* pParent, const QString& imagePath)
        : QWidget(pParent) {
    setStyleSheet("LaunchImage { background-color: #ff2020; }"
                  "QProgressBar { border: 0px; background-color: #202020; }"
                  "QProgressBar::chunk { background-color: white; }"
                 );

    QPixmap image(imagePath);
    QLabel *label = new QLabel(this);
    label->setPixmap(image);

    m_pProgressBar = new QProgressBar(this);
    m_pProgressBar->setTextVisible(false);
    m_pProgressBar->setMaximumWidth(image.width());
    m_pProgressBar->setMaximumHeight(3);

    QHBoxLayout* hbox = new QHBoxLayout(this);
    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->addStretch();
    vbox->addWidget(label);
    vbox->addWidget(m_pProgressBar);
    vbox->addStretch();
    hbox->addStretch();
    hbox->addLayout(vbox);
    hbox->addStretch();
}

LaunchImage::~LaunchImage() {
}

void LaunchImage::progress(int value) {
    m_pProgressBar->setValue(value);
}

void LaunchImage::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

