#include <skin/launchimage.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QStyleOption>
#include <QPainter>

LaunchImage::LaunchImage(QWidget* pParent, const QString& styleSheet)
        : QWidget(pParent) {
    if (styleSheet.isEmpty()) {
        setStyleSheet(
                "LaunchImage { background-color: #202020; }"
                "QLabel { "
                    "image: url(:/images/mixxx-icon-logo-symbolic.png);"
                    "padding:0;"
                    "margin:0;"
                    "border:none;"
                    "min-width: 208px;"
                    "min-height: 48px;"
                    "max-width: 208px;"
                    "max-height: 48px;"
                "}"
                "QProgressBar {"
                    "background-color: #202020; "
                    "border:none;"
                    "min-width: 208px;"
                    "min-height: 3px;"
                    "max-width: 208px;"
                    "max-height: 3px;"
                "}"
                "QProgressBar::chunk { background-color: #f3efed; }");
    } else {
        setStyleSheet(styleSheet);
    }

    QLabel *label = new QLabel(this);

    m_pProgressBar = new QProgressBar(this);
    m_pProgressBar->setTextVisible(false);

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

