#include "widget/whighlightinggroup.h"

#include <QLayout>
#include <QMap>
#include <QStylePainter>
#include <QStackedLayout>

#include "skin/skincontext.h"
#include "widget/wwidget.h"
#include "util/debug.h"
#include "widget/wpixmapstore.h"

namespace {
    const int kNoID = -1;
    const int kNoHighlight = 0;
}

WHighlightingGroup::WHighlightingGroup(QWidget* pParent)
        : WWidgetGroup(pParent),
          m_pPixmapBackHighlighted(nullptr),
          m_highlight(0),
          m_id(kNoID),
          m_highlightedId(kNoHighlight) {
    setObjectName("HighlightingGroup");
}

void WHighlightingGroup::setup(const QDomNode& node, const SkinContext& context) {
    WWidgetGroup::setup(node, context);

    // Set background pixmap for the highlighted state
    QDomElement backPathNodehighlighted =
            context.selectElement(node, "BackPathHighlighted");
    if (!backPathNodehighlighted.isNull()) {
        setPixmapBackgroundHighlighted(
                context.getPixmapSource(backPathNodehighlighted),
                context.selectScaleMode(backPathNodehighlighted, Paintable::TILE));
    }

    context.hasNodeSelectInt(node, "Id", &m_id);
}

void WHighlightingGroup::setPixmapBackgroundHighlighted(
        PixmapSource source, Paintable::DrawMode mode) {
    // Load background pixmap for the highlighted state
    m_pPixmapBackHighlighted = WPixmapStore::getPaintable(source, mode);
    if (!m_pPixmapBackHighlighted) {
        qDebug() << "WHighlightingGroup: Error loading background highlighted pixmap:" << source.getPath();
    }
}

void WHighlightingGroup::paintEvent(QPaintEvent* pe) {
    QFrame::paintEvent(pe);

    if (m_highlight == 1) {
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

int WHighlightingGroup::getHighlight() const {
    return m_highlight;
}

int WHighlightingGroup::getHighlightedId() const {
    return m_highlightedId;
}

void WHighlightingGroup::setHighlight(int highlight) {
    m_highlight = highlight;
    update();
    emit highlightChanged(m_highlight);
}

void WHighlightingGroup::setHighlightedId(int id) {
    m_highlightedId = id;
    if (m_id == id) {
        setHighlight(1);
    } else {
        setHighlight(0);
    }
    emit highlightedIdChanged(m_highlightedId);
}
