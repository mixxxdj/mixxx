// By Adam Davison
//  Heavily modified (hell, nearly rewritten) by Sean M. Pappalardo
// Code to listen to a control object and send a midi message based on the value

#ifndef MIDILEDHANDLER_H
#define MIDILEDHANDLER_H

#include "controlobject.h"
#include "mididevice.h"

class MidiLedHandler : QObject
{
    Q_OBJECT
public:
	MidiLedHandler(QString group, QString key, MidiDevice & midi, double min, double max,
		unsigned char status, unsigned char midino, unsigned char on, unsigned char off);
    ~MidiLedHandler();

    void update();

	static void createHandlers(QDomNode node, MidiDevice & midi);
    static void updateAll();
    static void destroyHandlers(MidiDevice* midi);
	static QList<MidiLedHandler*> allhandlers;

public slots:
	void controlChanged(double value);

private:
    MidiDevice* getMidiDevice() { return &m_midi; };
    
	MidiDevice& m_midi;
	double m_min;
	double m_max;
	ControlObject* m_cobj;
	unsigned char m_status;
	unsigned char m_midino;
	unsigned char m_on;
	unsigned char m_off;
	QMutex m_reentracyBlock;
};

#endif

