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
	MidiLedHandler(QString group, QString key, MidiObject& midi, double min, double max,
		unsigned char status, unsigned char midino, QString device, unsigned char on, unsigned char off);
    ~MidiLedHandler();

	static void createHandlers(QDomNode node, MidiObject& midi, QString device);
	static void destroyHandlers();
	static Q3PtrList<MidiLedHandler> allhandlers;

public slots:
	void controlChanged(double value);

private:
	MidiObject& m_midi;
	double m_min;
	double m_max;
	ControlObject* m_cobj;
	unsigned char m_status;
	unsigned char m_midino;
	QString m_device;
	unsigned char m_on;
	unsigned char m_off;
	unsigned char lastStatus;
	QMutex m_reentracyBlock;
};

#endif

