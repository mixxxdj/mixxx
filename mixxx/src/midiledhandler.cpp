#include "midiledhandler.h"
#include "wwidget.h"
//Added by qt3to4:
#include <Q3PtrList>
#include <QDebug>

Q3PtrList<MidiLedHandler> MidiLedHandler::allhandlers = Q3PtrList<MidiLedHandler>();

MidiLedHandler::MidiLedHandler(QString group, QString name, MidiObject * midi, double min,
                               double max, unsigned char status, unsigned char byte1)
    : m_min(min), m_max(max), m_midi(midi), m_status(status), m_byte1(byte1) {

    m_cobj = ControlObject::getControl(ConfigKey(group, name));
    

    //OMGWTFBBQ: Massive hack to temporarily fix LP #254564 for the 1.6.0 release. 
    //           Something's funky with our <lights> blocks handling? -- Albert 08/05/2008
    if (group.isEmpty() || name.isEmpty()) return;

    //m_cobj should never be null, so Q_ASSERT here to make sure that we hear about it if it is null.
    Q_ASSERT(m_cobj);
    connect(m_cobj, SIGNAL(valueChangedFromEngine(double)), this, SLOT(controlChanged(double)));
    connect(m_cobj, SIGNAL(valueChanged(double)), this, SLOT(controlChanged(double)));
}

MidiLedHandler::~MidiLedHandler() {
}

void MidiLedHandler::controlChanged(double value) {
    unsigned char m_byte2 = 0x00;
    if (value >= m_min && value <= m_max) { m_byte2 = 0x7f; }

    m_midi->sendShortMsg(m_status, m_byte1, m_byte2);
}

void MidiLedHandler::createHandlers(QDomNode node, MidiObject * midi) {

    if (!node.isNull() && node.isElement()) {
        QDomNode light = node.firstChild();

        while (!light.isNull()) {
            if(light.nodeName() == "light") {
                QString group = WWidget::selectNodeQString(light, "group");
                QString key = WWidget::selectNodeQString(light, "key");
    
                unsigned char status = (unsigned char)WWidget::selectNodeInt(light, "status");
                unsigned char midino = (unsigned char)WWidget::selectNodeInt(light, "midino");
				float min = 0.0f;
				float max = 1.0f;
				if (!light.firstChildElement("threshold").isNull()) {
					min = WWidget::selectNodeFloat(light, "threshold");
				}
				if (!light.firstChildElement("minimum").isNull()) {
					min = WWidget::selectNodeFloat(light, "minimum");
				}
				if (!light.firstChildElement("maximum").isNull()) {
					max = WWidget::selectNodeFloat(light, "maximum");
				}
    
                allhandlers.append(new MidiLedHandler(group, key, midi, min, max, status, midino));
            }
            light = light.nextSibling();
        }
    }
}

void MidiLedHandler::destroyHandlers() {
    allhandlers.setAutoDelete(true);
    allhandlers.clear();
}

