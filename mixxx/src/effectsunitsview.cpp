#include <QtCore>
#include <QtGui>

#include "effectsunitsview.h"
#include "widget/wskincolor.h"
#include "widget/wpixmapstore.h"

/* EffectsUnitsView
 * Create and place X EffectsUnitsWidget
 *  * Pass m_Controller
 *  * Pass docelement for skinning X
 *  * Asign ID
 *  Load Skin File (Skin for like 10 widgets.)
 *  Load User Preferences
 */
EffectsUnitsView::EffectsUnitsView(QWidget * parent) : QWidget(parent)
{
	m_EffectsUnitsPresetManager = new EffectsUnitsPresetManager();
	m_EffectsUnitsController = new EffectsUnitsController(m_EffectsUnitsPresetManager);
	m_EffectsUnitsSlots = new QList<EffectsUnitsSlot *>();

    setObjectName("EffectsUnits");
    m_pGridLayout = new QGridLayout();
    this->setLayout(m_pGridLayout);

    /* Opens up our skin file */
    QDomDocument skin("EffectsUnitsSkin");
    QFile file(WWidget::getPath("skin_fx.xml"));
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "FXUNITS: Could not open skin definition file: " << file.fileName();
    }
    if (!skin.setContent(&file))
    {
        qDebug() << "FXUNITS: Error parsing skin definition file: " << file.fileName();
    }
    file.close();
    qDebug() << "FXUNITS: Skin parsing done!";

    /* Gets the DOM of the Skin, so we're iterating on top of it.
     * When we get to a EffectsUnitsSlot node, we instantiate a
     * EffectsUnitsSlot with the firstchild of that node, and controller
     * pointer.
     */
    QDomNode node = skin.documentElement().firstChild();
    while (!node.isNull()){

    	qDebug() << "FXUNITS: Skin: " << node.nodeName();

    	/* Instantiates our EffectsUnitsSlots */
    	if (node.nodeName() == "EffectsUnitsSlot"){
    		EffectsUnitsSlot * fxslot = new EffectsUnitsSlot(this, m_EffectsUnitsController, node.firstChild());
    		m_EffectsUnitsSlots->append(fxslot);
    		m_pGridLayout->addWidget(fxslot);
    		fxslot->show();

    	/* Apply Background */
    	} else if (node.nodeName() == "Background"){
    		// TODO - apply background with Qlabel hack lol
    	}

    	/* Next node, please. */
    	node = node.nextSibling();
    }
}

EffectsUnitsView::~EffectsUnitsView()
{

}


