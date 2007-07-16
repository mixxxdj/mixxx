// By Adam Davison
// Code to listen to a control object and send a midi message based on the value

#ifndef MIDILEDHANDLER_H
#define MIDILEDHANDLER_H

#include "controlobject.h"
#include "midiobject.h"

#include <q3ptrlist.h>

class MidiLedHandler : QObject
{
    Q_OBJECT
public:
	MidiLedHandler(QString group, QString name, MidiObject* midi, double threshold,
		unsigned char status, unsigned char byte1);
    ~MidiLedHandler();

	static void createHandlers(QDomNode node, MidiObject* midi);
	static void destroyHandlers();
	static Q3PtrList<MidiLedHandler> allhandlers;

public slots:
	void controlChanged(double value);

private:
	MidiObject* m_midi;
	double m_threshold;
	ControlObject* m_cobj;
	unsigned char m_status;
	unsigned char m_byte1;

};

#endif

