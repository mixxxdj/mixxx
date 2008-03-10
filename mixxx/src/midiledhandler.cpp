#include "midiledhandler.h"
#include "wwidget.h"
//Added by qt3to4:
#include <Q3PtrList>

Q3PtrList<MidiLedHandler> MidiLedHandler::allhandlers = Q3PtrList<MidiLedHandler>();

MidiLedHandler::MidiLedHandler(QString group, QString name, MidiObject * midi, double threshold,
                               unsigned char status, unsigned char byte1)
    : m_threshold(threshold), m_midi(midi), m_status(status), m_byte1(byte1) {

    m_cobj = ControlObject::getControl(\
        ConfigKey(group, name));

    connect(m_cobj, SIGNAL(valueChangedFromEngine(double)), this, SLOT(controlChanged(double)));
    connect(m_cobj, SIGNAL(valueChanged(double)), this, SLOT(controlChanged(double)));
}

MidiLedHandler::~MidiLedHandler() {
}

void MidiLedHandler::controlChanged(double value) {
    unsigned char m_byte2 = 0x00;
    if (value >= m_threshold) { m_byte2 = 0x7f; }

    m_midi->sendShortMsg(m_status, m_byte1, m_byte2);
}

void MidiLedHandler::createHandlers(QDomNode node, MidiObject * midi) {

    if (!node.isNull() && node.isElement()) {
        QDomNode light = node.firstChild();

        while (!light.isNull()) {

            QString group = WWidget::selectNodeQString(light, "group");
            QString key = WWidget::selectNodeQString(light, "key");

            unsigned char status = (unsigned char)WWidget::selectNodeInt(light, "status");
            unsigned char midino = (unsigned char)WWidget::selectNodeInt(light, "midino");
            float threshold = WWidget::selectNodeFloat(light, "threshold");

            allhandlers.append(new MidiLedHandler(group, key, midi, threshold, status, midino));

            light = light.nextSibling();
        }
    }
}

void MidiLedHandler::destroyHandlers() {
    allhandlers.setAutoDelete(true);
    allhandlers.clear();
}

