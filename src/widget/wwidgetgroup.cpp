#include "widget/wwidgetgroup.h"

#include <QLayout>
#include <QMap>
#include <QStackedLayout>
#include <QStylePainter>

#include "moc_wwidgetgroup.cpp"
#include "skin/skincontext.h"
#include "util/debug.h"
#include "widget/wpixmapstore.h"
#include "widget/wwidget.h"

WWidgetGroup::WWidgetGroup(QWidget* pParent)
        : QFrame(pParent),
          WBaseWidget(this),
          m_pPixmapBack(nullptr),
          m_pPixmapBackHighlighted(nullptr),
          m_highlight(0) {
    setObjectName("WidgetGroup");
}

int WWidgetGroup::layoutSpacing() const {
    QLayout* pLayout = layout();
    return pLayout ? pLayout->spacing() : 0;
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

QRect WWidgetGroup::layoutContentsMargins() const {
    QLayout* pLayout = layout();
    QMargins margins = pLayout ? pLayout->contentsMargins() :
            contentsMargins();
    return QRect(margins.left(), margins.top(),
                 margins.right(), margins.bottom());
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

Qt::Alignment WWidgetGroup::layoutAlignment() const {
    QLayout* pLayout = layout();
    return pLayout ? pLayout->alignment() : Qt::Alignment();
}

void WWidgetGroup::setLayoutAlignment(int alignment) {
    //qDebug() << "WWidgetGroup::setLayoutAlignment" << alignment;

    QLayout* pLayout = layout();
    if (pLayout) {
        pLayout->setAlignment(static_cast<Qt::Alignment>(alignment));
    }
}

QLayout::SizeConstraint sizeConstraintFromString(const QString& constraint) {
    if (constraint.compare("SetDefaultConstraint", Qt::CaseInsensitive) == 0) {
        return QLayout::SetDefaultConstraint;
    } else if (constraint.compare("SetFixedSize", Qt::CaseInsensitive) == 0) {
        return QLayout::SetFixedSize;
    } else if (constraint.compare("SetMinimumSize", Qt::CaseInsensitive) == 0) {
        return QLayout::SetMinimumSize;
    } else if (constraint.compare("SetMaximumSize", Qt::CaseInsensitive) == 0) {
        return QLayout::SetMaximumSize;
    } else if (constraint.compare("SetMinAndMaxSize", Qt::CaseInsensitive) == 0) {
        return QLayout::SetMinAndMaxSize;
    } else if (constraint.compare("SetNoConstraint", Qt::CaseInsensitive) == 0) {
        return QLayout::SetNoConstraint;
    }
    return QLayout::SetDefaultConstraint;
}

void WWidgetGroup::setup(const QDomNode& node, const SkinContext& context) {
    setContentsMargins(0, 0, 0, 0);

    // Set background pixmap if available
    QDomElement backPathNode = context.selectElement(node, "BackPath");
    if (!backPathNode.isNull()) {
        setPixmapBackground(
                context.getPixmapSource(backPathNode),
                context.selectScaleMode(backPathNode, Paintable::TILE),
                context.getScaleFactor());
    }

    // Set background pixmap for the highlighted state
    QDomElement backPathNodeHighlighted =
            context.selectElement(node, "BackPathHighlighted");
    if (!backPathNodeHighlighted.isNull()) {
        setPixmapBackgroundHighlighted(
                context.getPixmapSource(backPathNodeHighlighted),
                context.selectScaleMode(backPathNodeHighlighted, Paintable::TILE),
                context.getScaleFactor());
    }

    QLayout* pLayout = nullptr;
    QString layout;
    if (context.hasNodeSelectString(node, "Layout", &layout)) {
        if (layout == "vertical") {
            pLayout = new QVBoxLayout();
        } else if (layout == "horizontal") {
            pLayout = new QHBoxLayout();
        } else if (layout == "stacked") {
            auto* pStackedLayout = new QStackedLayout();
            pStackedLayout->setStackingMode(QStackedLayout::StackAll);
            pLayout = pStackedLayout;
            // Adding a zero-size dummy widget as index 0 here before
            // any child is added in the xml template works around
            // https://bugs.launchpad.net/mixxx/+bug/1627859
            QWidget *dummyWidget = new QWidget();
            dummyWidget->setFixedSize(0, 0);
            pLayout->addWidget(dummyWidget);
        }

        // Set common layout parameters.
        if (pLayout != nullptr) {
            pLayout->setSpacing(0);
            pLayout->setContentsMargins(0, 0, 0, 0);
            pLayout->setAlignment(Qt::AlignCenter);
        }
    }

    QString sizeConstraintStr;
    if (pLayout && context.hasNodeSelectString(node, "SizeConstraint", &sizeConstraintStr)) {
        pLayout->setSizeConstraint(sizeConstraintFromString(sizeConstraintStr));
    }

    if (pLayout) {
        setLayout(pLayout);
    }
}

void WWidgetGroup::setPixmapBackground(
        const PixmapSource& source,
        Paintable::DrawMode mode,
        double scaleFactor) {
    // Load background pixmap
    m_pPixmapBack = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (!m_pPixmapBack) {
        qWarning() << "WWidgetGroup: Error loading background pixmap:"
                 << source.getPath();
    }
}

void WWidgetGroup::setPixmapBackgroundHighlighted(
        const PixmapSource& source,
        Paintable::DrawMode mode,
        double scaleFactor) {
    // Load background pixmap for the highlighted state
    m_pPixmapBackHighlighted = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (!m_pPixmapBackHighlighted) {
        qWarning() << "WWidgetGroup: Error loading background highlighted pixmap:"
                 << source.getPath();
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

    if (m_highlight > 0) {
        if (m_pPixmapBackHighlighted) {
            QStylePainter p(this);
            m_pPixmapBackHighlighted->draw(rect(), &p);
        }
    } else {
        if (m_pPixmapBack) {
            QStylePainter p(this);
            m_pPixmapBack->draw(rect(), &p);
        }
    }
}

void WWidgetGroup::resizeEvent(QResizeEvent* re) {
    // Paint things styled by style sheet
    QFrame::resizeEvent(re);
}

bool WWidgetGroup::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QFrame::event(pEvent);
}

void WWidgetGroup::fillDebugTooltip(QStringList* debug) {
    WBaseWidget::fillDebugTooltip(debug);
    *debug << QString("LayoutAlignment: %1").arg(toDebugString(layoutAlignment()))
           << QString("LayoutContentsMargins: %1").arg(toDebugString(layoutContentsMargins()))
           << QString("LayoutSpacing: %1").arg(layoutSpacing());
}

int WWidgetGroup::getHighlight() const {
    return m_highlight;
}

void WWidgetGroup::setHighlight(int highlight) {
    if (m_highlight == highlight) {
        return;
    }
    m_highlight = highlight;
    style()->unpolish(this);
    style()->polish(this);
    update();
    emit highlightChanged(m_highlight);
}
