/***************************************************************************
                          wstarrating.cpp  -  description
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2003 by Tue Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "widget/wstarrating.h"

#include "library/starrating.h"


// #include "widget/wskincolor.h"

// WStarRating::WStarRating(QWidget* pParent)
        // : WBaseWidget(this) {
// }

WStarRating::WStarRating(const char* group,
                               ConfigObject<ConfigValue>* pConfig,
                               QWidget* pParent)
        : m_pGroup(group),
          m_pConfig(pConfig) {
    // setAcceptDrops(true);
}


WStarRating::~WStarRating() {
}

void WStarRating::setup(QDomNode node, const SkinContext& context) {
	
	// lecture du noeud xml
    m_property = context.selectString(node, "Property");
	
	// association du track
	// m_pCurrentTrack = track; // TrackPointer track
	// connect(track.data(), SIGNAL(changed(TrackInfoObject*)),
			// this, SLOT(updateRating(TrackInfoObject*)));
	
	// premier affichage de la valeur
	// updateRating(track.data());
	
    // Used by delegates (e.g. StarDelegate) to tell when the mouse enters a
    // cell.
    setMouseTracking(true);
	
	/** /
    // Colors
    QPalette pal = palette(); //we have to copy out the palette to edit it since it's const (probably for threadsafety)
    if (context.hasNode(node, "BgColor")) {
        m_qBgColor.setNamedColor(context.selectString(node, "BgColor"));
        pal.setColor(this->backgroundRole(), WSkinColor::getCorrectColor(m_qBgColor));
        setAutoFillBackground(true);
    }
    m_qFgColor.setNamedColor(context.selectString(node, "FgColor"));
    pal.setColor(this->foregroundRole(), WSkinColor::getCorrectColor(m_qFgColor));
    setPalette(pal);

    // Text
    if (context.hasNode(node, "Text"))
        m_qsText = context.selectString(node, "Text");
    setText(m_qsText);

    // Font size
    if (context.hasNode(node, "FontSize")) {
        int fontsize = context.selectString(node, "FontSize").toInt();
        setFont( QFont("Helvetica",fontsize,QFont::Normal) );
    }

    // Alignment
    if (context.hasNode(node, "Alignment")) {
        if (context.selectString(node, "Alignment").toLower() == "right") {
            setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        } else if (context.selectString(node, "Alignment").toLower() == "center") {
            setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        } else if (context.selectString(node, "Alignment").toLower() == "left") {
            setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }
	/**/
}

bool WStarRating::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    // return QLabel::event(pEvent);
    return true;
}

void WStarRating::fillDebugTooltip(QStringList* debug) {
    // WBaseWidget::fillDebugTooltip(debug);
    // *debug << QString("Text: \"%1\"").arg(text());
}

void WStarRating::slotTrackLoaded(TrackPointer track) {
    if (track) {
        m_pCurrentTrack = track;
        connect(track.data(), SIGNAL(changed(TrackInfoObject*)),
                this, SLOT(updateRating(TrackInfoObject*)));
        updateRating(track.data());
    }
}

void WStarRating::slotTrackUnloaded(TrackPointer track) {
    Q_UNUSED(track);
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.data(), 0, this, 0);
    }
    m_pCurrentTrack.clear();
    // setText("");
}

void WStarRating::updateRating(TrackInfoObject*) {
    if (m_pCurrentTrack) {
        QVariant property = m_pCurrentTrack->property(m_property.toAscii().constData());
        // if (property.isValid() && qVariantCanConvert<QString>(property)) {
            // setText(property.toString());
        // }
		// if (qVariantCanConvert<int>(value))
			// value =  QString("(%1)").arg(value.toInt());
        if (property.isValid()) {
			
			StarRating starRating = qVariantValue<StarRating>(
				m_pCurrentTrack->property(m_property.toAscii().constData()) );
			// pTrack->setRating(starRating.starCount());
			m_pCurrentTrack->setRating(starRating.starCount());
        }
    }
}

// void 	render ( QPainter * painter, const QPoint & targetOffset = QPoint(), const QRegion & sourceRegion = QRegion(), RenderFlags renderFlags = RenderFlags( DrawWindowBackground | DrawChildren ) )

void WStarRating::render (
		QPainter * painter,
		const QPoint & targetOffset /*= QPoint()*/,
		const QRegion & sourceRegion /*= QRegion()*/,
		RenderFlags renderFlags /*= RenderFlags( DrawWindowBackground | DrawChildren )*/ ){
	
	// todo : render the starrating
	// void StarRating::paint(QPainter *painter, const QRect &rect, const QPalette &palette,
							// EditMode mode, bool isSelected) const
	// WStarRating.paint( painter );
	
}
