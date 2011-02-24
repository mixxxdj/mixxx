/*
 * effectsunitspreset.h
 *
 *  Created on: Aug 10, 2010
 *      Author: bruno
 */

#ifndef EFFECTSUNITSPRESET_H_
#define EFFECTSUNITSPRESET_H_

#include <QtCore>
#include <QtXml>

class EffectsUnitsPreset {
  public:
    EffectsUnitsPreset(QDomNode Preset);
    virtual ~EffectsUnitsPreset();

    QString presetFor();
    QDomDocument toXML();
    QList<int>* getBindedPortIndex();
    int getWetDryPortIndex();
    void setBindedPortsFromXML(QDomNode node);

  private:
    // plugin name
    QString * m_pPresetFor;
    // Wet/Dry port Index, -1 if none
    int m_WetDryPortIndex;
    // Knob <--> Port Mappings
    QList<int>* m_pBindedPortIndex;
};

#endif /* EFFECTSUNITSPRESET_H_ */
