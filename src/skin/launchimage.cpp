
#include <skin/launchimage.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>


LaunchImage::LaunchImage(QWidget* pParent)
        : QWidget(pParent) {
    setStyleSheet("background-color: #202020;");

    QPixmap pic(":/images/mixxx-icon-logo-symbolic.png");
    QLabel *label = new QLabel(this);
    label->setPixmap(pic);

    m_pProgressBar = new QProgressBar(this);
    m_pProgressBar->setTextVisible(false);
    m_pProgressBar->setMaximumWidth(pic.width());
    m_pProgressBar->setMaximumHeight(3);
    m_pProgressBar->setStyleSheet(
            "QProgressBar::chunk {"
                "background-color: white;"
            "}"
            "QProgressBar {"
                "border: 0px"
            "}");

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

