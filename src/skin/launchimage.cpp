
#include <skin/launchimage.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>


LaunchImage::LaunchImage(QWidget* pParent)
        : QWidget(pParent) {
    setStyleSheet("background-color: #202020;");
    QPixmap pic(":/images/mixxx-icon-symbolic.png");
    QLabel *label = new QLabel(this);
    label->setPixmap(pic);

    QHBoxLayout* hbox = new QHBoxLayout(this);
    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->addStretch();
    vbox->addWidget(label);
    vbox->addStretch();
    hbox->addStretch();
    hbox->addLayout(vbox);
    hbox->addStretch();
}

LaunchImage::~LaunchImage() {
}

