// By Adam Davison
// Code to listen to a control object and send a midi message based on the value

#ifndef MIDILEDHANDLER_H
#define MIDILEDHANDLER_H

#include "controlobject.h"
#include "mididevice.h"

#include <q3ptrlist.h>

class MidiLedHandler : QObject
{
    Q_OBJECT
public:
	MidiLedHandler(QString group, QString key, MidiDevice & midi, double min, double max,
		unsigned char status, unsigned char midino, unsigned char on, unsigned char off);
    ~MidiLedHandler();

	static void createHandlers(QDomNode node, MidiDevice & midi);
	static void destroyHandlers();
	static Q3PtrList<MidiLedHandler> allhandlers;

public slots:
	void controlChanged(double value);

private:
	MidiDevice& m_midi;
	double m_min;
	double m_max;
	ControlObject* m_cobj;
	unsigned char m_status;
	unsigned char m_midino;
	unsigned char m_on;
	unsigned char m_off;
	unsigned char lastStatus;
	QMutex m_reentracyBlock;
};

#endif

