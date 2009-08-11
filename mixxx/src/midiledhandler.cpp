#include "midiledhandler.h"
#include "widget/wwidget.h"
//Added by qt3to4:
#include <Q3PtrList>
#include <QDebug>

Q3PtrList<MidiLedHandler> MidiLedHandler::allhandlers = Q3PtrList<MidiLedHandler>();

MidiLedHandler::MidiLedHandler(QString group, QString key, MidiObject & midi, double min,
                               double max, unsigned char status, unsigned char midino, QString device, unsigned char on, unsigned char off)
    : m_min(min), m_max(max), m_midi(midi), m_status(status), m_midino(midino), m_device(device), m_on(on), m_off(off) {

    //OMGWTFBBQ: Massive hack to temporarily fix LP #254564 for the 1.6.0 release.
    //           Something's funky with our <lights> blocks handling? -- Albert 08/05/2008
    if (group.isEmpty() || key.isEmpty()) return;

    m_cobj = ControlObject::getControl(ConfigKey(group, key));

    //m_cobj should never be null, so Q_ASSERT here to make sure that we hear about it if it is null.
    //Q_ASSERT(m_cobj);
    QByteArray err_tmp = QString("Invalid config group: '%1', name: '%2'").arg(group).arg(key).toAscii();
    Q_ASSERT_X(m_cobj, "MidiLedHandler", err_tmp);

    connect(m_cobj, SIGNAL(valueChangedFromEngine(double)), this, SLOT(controlChanged(double)), Qt::DirectConnection);
    connect(m_cobj, SIGNAL(valueChanged(double)), this, SLOT(controlChanged(double)), Qt::DirectConnection);
}

MidiLedHandler::~MidiLedHandler() {
}

void MidiLedHandler::controlChanged(double value) {
    //Guh, the valueChangedFromEngine and valueChanged signals can occur simultaneously because we're
    //using Qt::DirectConnection. We have to block by hand to prevent re-entrancy. We can't use 
    //Qt::BlockingQueuedConnection because we don't have an event loop in some of the threads that
    //create MidiLedHandlers. (The underlying code is messy - On first run, the LED handlers get created
    //in the main thread, and then after you load a new binding, they get created from the Midi thread.)
    // - Albert 04/19/2009
    m_reentracyBlock.lock();
    unsigned char m_byte2 = m_off;
    if (value >= m_min && value <= m_max) { m_byte2 = m_on; }

    if (lastStatus!=m_byte2) {
        lastStatus=m_byte2;
         if (m_byte2 != 0xff) {
            // qDebug() << "MIDI bytes:" << m_status << ", " << m_midino << ", " << m_byte2 ;
            m_midi.sendShortMsg(m_status, m_midino, m_byte2);
        }
    }
    m_reentracyBlock.unlock();
}

void MidiLedHandler::createHandlers(QDomNode node, MidiObject & midi, QString device) {
    if (!node.isNull() && node.isElement()) {
        QDomNode light = node;
        while (!light.isNull()) {
            if(light.nodeName() == "output") {
                QString group = WWidget::selectNodeQString(light, "group");
                QString key = WWidget::selectNodeQString(light, "key");

                unsigned char status = (unsigned char)WWidget::selectNodeInt(light, "status");
                unsigned char midino = (unsigned char)WWidget::selectNodeInt(light, "midino");
                unsigned char on = 0x7f;    // Compatible with Hercules and others
                unsigned char off = 0x00;
                float min = 0.0f;
                float max = 1.0f;
                if (!light.firstChildElement("on").isNull()) {
                    on = (unsigned char)WWidget::selectNodeInt(light, "on");
                }
                if (!light.firstChildElement("off").isNull()) {
                    off = (unsigned char)WWidget::selectNodeInt(light, "off");
                }
                if (!light.firstChildElement("threshold").isNull()) { //Deprecated as of 1.7.0
                    min = WWidget::selectNodeFloat(light, "threshold");
                }
                if (!light.firstChildElement("minimum").isNull()) {
                    min = WWidget::selectNodeFloat(light, "minimum");
                }
                if (!light.firstChildElement("maximum").isNull()) {
                    max = WWidget::selectNodeFloat(light, "maximum");
                }
                qDebug() << "Creating LED handler hook for:" << group << key << "between"<< min << "and" << max << "to midi out:" << status << midino << "on" << device << "on/off:" << on << off;
                allhandlers.append(new MidiLedHandler(group, key, midi, min, max, status, midino, device, on, off));
            }
            light = light.nextSibling();
        }
    }
}

void MidiLedHandler::destroyHandlers() {
    allhandlers.setAutoDelete(true);
    allhandlers.clear();
}

