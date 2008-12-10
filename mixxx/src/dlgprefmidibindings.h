/***************************************************************************
                          dlgprefmidibindings.h  -  description
                             -------------------
    begin                : Sat Jun 21 2008
    copyright            : (C) 2008 by Tom Care
    email                : psyc0de@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DLGPREFMIDIBINDINGS_H_
#define DLGPREFMIDIBINDINGS_H_

#include <QtGui>
#include "ui_dlgprefmidibindingsdlg.h"
#include "configobject.h"
#include "midiledhandler.h"
#include "midiobject.h"

class DlgPrefMidiBindings : public QWidget, public Ui::DlgPrefMidiBindingsDlg  {
    Q_OBJECT
public:
    DlgPrefMidiBindings(QWidget *parent, MidiObject *midi, ConfigObject<ConfigValue> *pConfig);
    ~DlgPrefMidiBindings();


public slots:
    void slotUpdate();
    void slotApply();
    void slotSingleLearnToggle();
    void slotGroupLearnToggle();
    void slotImportXML();
    void slotExportXML();
    void slotClear();
    void slotRemoveBinding();
    void slotAddBinding();
//    void slotChangeBinding();
//    void slotAdvancedOptions();
    void singleLearn(ConfigValueMidi *value, QString device);
    void groupLearn(ConfigValueMidi *value, QString device);

    void slotTestOutputBinding();
    void slotClearOutputBindings();
    void slotAddOutputBinding();
    void slotRemoveOutputBinding();

private:
	void setRowBackground(int row, QColor color);
	void addRow(QString device, QString group, QString key, QString controltype, QString miditype, QString midino, QString midichan, QString option);
	void addOutputRow(QString outputType, QString group, QString key, QString min, QString max, QString status, QString midino, QString device, QString on, QString off);
    void removeSelectedBindings(QTableWidget* table);
	void loadPreset(QString path);
	void loadPreset(QDomElement root);
	void savePreset(QString path);
	void applyPreset();
	void clearPreset();
	void clearTable();
	void clearOutputTable();
	void buildDomElement();
	QStringList getControlKeyList();
	bool singleLearning;
	bool groupLearning;
	int currentGroupRow;
	QDomElement m_pBindings;
    MidiObject *m_pMidi;
    ConfigObject<ConfigValue> *m_pConfig;
    ConfigObject<ConfigValueMidi> *m_pMidiConfig;
};

#endif /*DLGPREFMIDIBINDINGS_H_*/
