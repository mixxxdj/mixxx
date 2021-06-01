#include "skin/legacy/launchimage.h"

#include <util/assert.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QProgressBar>
#include <QStyleOption>
#include <QVBoxLayout>

#include "moc_launchimage.cpp"

LaunchImage::LaunchImage(QWidget* pParent, const QString& styleSheet)
        : QWidget(pParent) {
    if (styleSheet.isEmpty()) {
        setStyleSheet(
                "LaunchImage { background-color: #202020; }"
                "QLabel { "
                "background-image: url(:/images/mixxx-icon-logo-symbolic.svg);"
                "padding:0;"
                "margin:0;"
                "border:none;"
                "min-width: 236px;"
                "min-height: 48px;"
                "max-width: 236px;"
                "max-height: 48px;"
                "}"
                "QProgressBar {"
                "background-color: #202020; "
                "border:none;"
                "min-width: 236px;"
                "min-height: 3px;"
                "max-width: 236px;"
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
    QVBoxLayout* vbox = new QVBoxLayout();
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

void LaunchImage::progress(int value, const QString& serviceName) {
    m_pProgressBar->setValue(value);
    // TODO: show serviceName
    Q_UNUSED(serviceName);
}

void LaunchImage::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
