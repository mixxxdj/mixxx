#include "widget/wwidgetgroup.h"

#include <QLayout>
#include <QStylePainter>

#include "widget/wwidget.h"
#include "widget/wpixmapstore.h"
#include "xmlparse.h"

WWidgetGroup::WWidgetGroup(QWidget* pParent)
        : QGroupBox(pParent),
          m_pPixmapBack(NULL) {
    setObjectName("WidgetGroup");
}

WWidgetGroup::~WWidgetGroup() {
}

void WWidgetGroup::setLayoutSpacing(int spacing) {
    //qDebug() << "WWidgetGroup::setSpacing" << spacing;
    if (spacing < 0) {
        qDebug() << "WWidgetGroup: Invalid spacing:" << spacing;
        return;
    }
    QLayout* pLayout = layout();
    if (pLayout) {
        pLayout->setSpacing(spacing);
    }
}

void WWidgetGroup::setLayoutContentsMargins(QRect rectMargins) {
    // qDebug() << "WWidgetGroup::setLayoutContentsMargins" << rectMargins.x()
    //          << rectMargins.y() << rectMargins.width() << rectMargins.height();

    if (rectMargins.x() < 0 || rectMargins.y() < 0 ||
            rectMargins.width() < 0 || rectMargins.height() < 0) {
        qDebug() << "WWidgetGroup: Invalid ContentsMargins rectangle:"
                 << rectMargins;
        return;
    }

    setContentsMargins(rectMargins.x(), rectMargins.y(),
                       rectMargins.width(), rectMargins.height());
    QLayout* pLayout = layout();
    if (pLayout) {
        pLayout->setContentsMargins(rectMargins.x(), rectMargins.y(),
                                    rectMargins.width(), rectMargins.height());
    }
}

void WWidgetGroup::setLayoutAlignment(int alignment) {
    //qDebug() << "WWidgetGroup::setLayoutAlignment" << alignment;

    QLayout* pLayout = layout();
    if (pLayout) {
        pLayout->setAlignment(static_cast<Qt::Alignment>(alignment));
    }
}

void WWidgetGroup::setup(QDomNode node, const SkinContext& context) {
    setContentsMargins(0, 0, 0, 0);

    // Set background pixmap if available
    if (context.hasNode(node, "BackPath")) {
        setPixmapBackground(WWidget::getPath(context.selectString(node, "BackPath")));
    }

    QLayout* pLayout = NULL;
    if (context.hasNode(node, "Layout")) {
        QString layout = context.selectString(node, "Layout");
        if (layout == "vertical") {
            pLayout = new QVBoxLayout();
            pLayout->setSpacing(0);
            pLayout->setContentsMargins(0, 0, 0, 0);
            pLayout->setAlignment(Qt::AlignCenter);
        } else if (layout == "horizontal") {
            pLayout = new QHBoxLayout();
            pLayout->setSpacing(0);
            pLayout->setContentsMargins(0, 0, 0, 0);
            pLayout->setAlignment(Qt::AlignCenter);
        }
    }

    if (pLayout) {
        setLayout(pLayout);
    }
}

void WWidgetGroup::setPixmapBackground(const QString &filename) {
    // Load background pixmap
    m_pPixmapBack = WPixmapStore::getPaintable(filename);
    if (!m_pPixmapBack) {
        qDebug() << "WWidgetGroup: Error loading background pixmap:" << filename;
    }
}

void WWidgetGroup::addWidget(QWidget* pChild) {
    QLayout* pLayout = layout();
    if (pLayout && pChild) {
        pLayout->addWidget(pChild);
    }
}

void WWidgetGroup::paintEvent(QPaintEvent* pe) {
    QGroupBox::paintEvent(pe);

    if (m_pPixmapBack) {
        QStylePainter p(this);
        m_pPixmapBack->draw(rect(), &p);
    }
}

void WWidgetGroup::resizeEvent(QResizeEvent* re) {
    // Paint things styled by style sheet
    QGroupBox::resizeEvent(re);
}
