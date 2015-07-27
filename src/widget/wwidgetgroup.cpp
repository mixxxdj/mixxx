#include "widget/wwidgetgroup.h"

#include <QLayout>
#include <QMap>
#include <QStylePainter>

#include "widget/wwidget.h"
#include "widget/wpixmapstore.h"
#include "xmlparse.h"

WWidgetGroup::WWidgetGroup(QWidget* pParent)
        : QFrame(pParent),
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

void WWidgetGroup::setup(QDomNode node) {
    setContentsMargins(0, 0, 0, 0);

    // Set background pixmap if available
    if (!WWidget::selectNode(node, "BackPath").isNull()) {
        setPixmapBackground(WWidget::getPath(WWidget::selectNodeQString(node, "BackPath")));
    }

    QLayout* pLayout = NULL;
    if (!XmlParse::selectNode(node, "Layout").isNull()) {
        QString layout = XmlParse::selectNodeQString(node, "Layout");
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

    if (pLayout && !XmlParse::selectNode(node, "SizeConstraint").isNull()) {
        QMap<QString, QLayout::SizeConstraint> constraints;
        constraints["SetDefaultConstraint"] = QLayout::SetDefaultConstraint;
        constraints["SetFixedSize"] = QLayout::SetFixedSize;
        constraints["SetMinimumSize"] = QLayout::SetMinimumSize;
        constraints["SetMaximumSize"] = QLayout::SetMaximumSize;
        constraints["SetMinAndMaxSize"] = QLayout::SetMinAndMaxSize;
        constraints["SetNoConstraint"] = QLayout::SetNoConstraint;

        QString sizeConstraintStr = XmlParse::selectNodeQString(node, "SizeConstraint");
        if (constraints.contains(sizeConstraintStr)) {
            pLayout->setSizeConstraint(constraints[sizeConstraintStr]);
        } else {
            qDebug() << "Could not parse SizeConstraint:" << sizeConstraintStr;
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
    QFrame::paintEvent(pe);

    if (m_pPixmapBack) {
        QStylePainter p(this);
        m_pPixmapBack->draw(rect(), &p);
    }
}

void WWidgetGroup::resizeEvent(QResizeEvent* re) {
    // Paint things styled by style sheet
    QFrame::resizeEvent(re);
}
