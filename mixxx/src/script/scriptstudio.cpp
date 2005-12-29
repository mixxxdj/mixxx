#include "scriptstudio.h"
#include "macrolist.h"
#include "macrolistitem.h"
#include <qwidget.h>
#include <qpopupmenu.h>
#include <qdockwindow.h>
#include <qlistview.h>
#include <qmessagebox.h>

#define ID_REC 1000
#define ID_STOP 1001

ScriptStudio::ScriptStudio(ScriptEngine* model, QWidget* parent,
			const char* name) : QMainWindow(parent, name) {
	m_model = model;
	m_current = 0;
	m_edit = new QTextEdit(this, "Editor Pane");
	m_edit->setFocus();

	m_rec = ScriptRecorder();

	m_edit->setEnabled(false);
	setCentralWidget(m_edit);
	resize(600,400);
	
	QPopupMenu* script = new QPopupMenu(this);
	menuBar()->insertItem("&Macro", script);
	script->insertItem("&New", this, SLOT(newScript()));
	script->insertItem("&Record", this, SLOT(recordScript()), 0, ID_REC);
	script->insertItem("&Stop Record", this, SLOT(stopRecord()), 0, ID_STOP);
	script->insertItem("&Play", this, SLOT(playMacro()));
	script->insertItem("&Delete", this, SLOT(deleteMacro()));
	
	menuBar()->setItemEnabled(ID_STOP, false);
	
	QDockWindow* left = new QDockWindow(this);
	m_mlist = new MacroList(m_model, left);
	left->setWidget(m_mlist);
	addDockWindow(left, "Macro List Panel", Left); 

	connect(m_mlist, SIGNAL(selectionChanged()), this, SLOT(changeScript()));
	connect(m_edit, SIGNAL(textChanged()), this, SLOT(editScript()));
	
	//show();
}

ScriptStudio::~ScriptStudio() {
}

void ScriptStudio::newScript() {
	m_model->newMacro();
	m_mlist->repaint();
	m_model->saveMacros();
}

void ScriptStudio::changeScript() {
	//int pos = m_mlist->itemPos(m_mlist->selectedItem());
	//m_current = m_model->getMacro(pos);
	m_model->saveMacros();
	MacroListItem* item = (MacroListItem*)m_mlist->selectedItem();
	if (item == NULL) {
		return;
	}
	m_current = item->getMacro();
	m_edit->setText(m_current->getScript());
	m_edit->setEnabled(true);
}

void ScriptStudio::editScript() {
	if (m_current != NULL) {
		m_current->setScript(m_edit->text());
	} else {
		newScript();
		m_current->setScript(m_edit->text());
	}
}

void ScriptStudio::recordScript() {
	menuBar()->setItemEnabled(ID_REC, false);
	menuBar()->setItemEnabled(ID_STOP, true);
	m_rec.startRecord();
}

void ScriptStudio::stopRecord() {
	m_rec.stopRecord();
	menuBar()->setItemEnabled(ID_REC, true);
	menuBar()->setItemEnabled(ID_STOP, false);
	Macro* nmacro = new Macro("Recorded Macro", *m_rec.getMacro());
	m_model->addMacro(nmacro);
	m_mlist->repaint();
	m_rec.reset();
	m_model->saveMacros();
}

void ScriptStudio::playMacro() {
	m_model->saveMacros();
        MacroListItem* item = (MacroListItem*)m_mlist->selectedItem();
        if (item == NULL) {
                return;
        }
        m_current = item->getMacro();
        m_model->executeScript(m_current->getScript());
}

void ScriptStudio::deleteMacro() {
        MacroListItem* item = (MacroListItem*)m_mlist->selectedItem();
        if (item == NULL) {
	        return;
	}
	
	QMessageBox msg("Deleting Macro", "Really delete macro?",
			QMessageBox::Warning, QMessageBox::Yes,
			QMessageBox::No, QMessageBox::NoButton);
	if (msg.exec() != QMessageBox::Yes) {
		return;
	}
	
	m_current = item->getMacro();
	m_model->deleteMacro(m_current);
	m_mlist->repaint();
}

void ScriptStudio::showStudio() {
	show();
}
