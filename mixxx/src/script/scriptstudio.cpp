#include "scriptstudio.h"
#include "macrolist.h"
#include "macrolistitem.h"
#include <qwidget.h>
#include <q3popupmenu.h>
#include <q3dockwindow.h>
#include <qlistview.h>
#include <qmessagebox.h>

#define ID_REC 1000
#define ID_STOP 1001

#define ID_LUA 2000
#define ID_PYTHON 2001

ScriptStudio::ScriptStudio(ScriptEngine* model, QWidget* parent,
			const char* name) : Q3MainWindow(parent, name) {
	m_model = model;
	m_current = 0;
	m_edit = new QTextEdit(this, "Editor Pane");
	m_edit->setFocus();

	m_rec = model->getRecorder();

	m_edit->setEnabled(false);
	setCentralWidget(m_edit);
	resize(600,400);
	
	Q3PopupMenu* script = new Q3PopupMenu(this);
	Q3PopupMenu* newtype = new Q3PopupMenu(script);
	Q3PopupMenu* lang = new Q3PopupMenu(this);

	lang->setCheckable(true);
	menuBar()->insertItem("&Macro", script);
//	script->insertItem("&New", this, SLOT(newScript()));
	script->insertItem("&New", newtype);
#ifdef __LUA__
	script->insertItem("&Record", this, SLOT(recordScript()), 0, ID_REC);
	script->insertItem("&Stop Record", this, SLOT(stopRecord()), 0, ID_STOP);
#endif
	script->insertItem("&Play", this, SLOT(playMacro()));
	script->insertItem("&Delete", this, SLOT(deleteMacro()));

	menuBar()->insertItem("&Language", lang);
#ifdef __LUA__
	lang->insertItem("&Lua", this, SLOT(setLangLua()), 0, ID_LUA);
#endif
#ifdef __PYTHON__
	lang->insertItem("&Python", this, SLOT(setLangPython()), 0, ID_PYTHON);
#endif
	
	menuBar()->setItemEnabled(ID_STOP, false);
	menuBar()->setItemChecked(ID_LUA, false);
#ifdef __LUA__
	newtype->insertItem("&Lua Macro", this, SLOT(newLuaScript()));
#endif
#ifdef __PYTHON__
	newtype->insertItem("&Python Macro", this, SLOT(newPythonScript()));
#endif
	
	Q3DockWindow* left = new Q3DockWindow(this);
	m_mlist = new MacroList(m_model, left);
	left->setWidget(m_mlist);
	//addDockWindow(left, "Macro List Panel", left->Left); 

	connect(m_mlist, SIGNAL(selectionChanged()), this, SLOT(changeScript()));
	connect(m_edit, SIGNAL(textChanged()), this, SLOT(editScript()));
	
	//show();
}

ScriptStudio::~ScriptStudio() {
}

void ScriptStudio::newPythonScript() {
#ifdef __PYTHON__
        m_model->newMacro(Macro::LANG_PYTHON);
        m_mlist->repaint();
        m_model->saveMacros();
#endif
}

// Can't comment out whole function, the moc thing is a bit too naive
void ScriptStudio::newLuaScript() {
#ifdef __LUA__
	m_model->newMacro(Macro::LANG_LUA);
	m_mlist->repaint();
	m_model->saveMacros();
#endif
}

void ScriptStudio::changeScript() {
	//int pos = m_mlist->itemPos(m_mlist->selectedItem());
	//m_current = m_model->getMacro(pos);
	m_model->saveMacros();
/*	MacroListItem* item = (MacroListItem*)m_mlist->selectedItem();
	if (item == NULL) {
		m_current = 0;
		m_edit->setText("");
		m_edit->setEnabled(false);
		return;
	}
	m_current = item->getMacro();
	m_edit->setText(m_current->getScript());
	m_edit->setEnabled(true);
	updateLangMenu();*/
}

void ScriptStudio::editScript() {
	if (m_current != NULL) {
		m_current->setScript(m_edit->text());
	}
}

void ScriptStudio::recordScript() {
	menuBar()->setItemEnabled(ID_REC, false);
	menuBar()->setItemEnabled(ID_STOP, true);
	m_rec->startRecord();
}

void ScriptStudio::stopRecord() {
	qDebug("1");
	m_rec->stopRecord();
	qDebug("2");
	menuBar()->setItemEnabled(ID_REC, true);
	qDebug("3");
	menuBar()->setItemEnabled(ID_STOP, false);
	qDebug("4");
	Macro* nmacro = m_rec->getMacro();
	qDebug("5");
	m_model->addMacro(nmacro);
	qDebug("6");
	m_mlist->repaint();
	qDebug("7");
	m_rec->reset();
	qDebug("8");
	m_model->saveMacros();
	qDebug("9");
}

void ScriptStudio::playMacro() {
	m_model->saveMacros();
/*        MacroListItem* item = (MacroListItem*)m_mlist->selectedItem();
        if (item == NULL) {
                return;
        }
        m_current = item->getMacro();
        m_model->executeMacro(m_current);*/
}

void ScriptStudio::deleteMacro() {
/*        MacroListItem* item = (MacroListItem*)m_mlist->selectedItem();
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
	m_mlist->repaint();*/
}

void ScriptStudio::showStudio() {
	show();
}

void ScriptStudio::setLangLua() {
#ifdef __LUA__
	if (m_current != NULL) {
		m_current->setLang(Macro::LANG_LUA);
	}
	updateLangMenu();
#endif
}

void ScriptStudio::setLangPython() {
#ifdef __PYTHON__
	if (m_current != NULL) {
		m_current->setLang(Macro::LANG_PYTHON);
	}
	updateLangMenu();
#endif
}

void ScriptStudio::updateLangMenu() {
	menuBar()->setItemChecked(ID_LUA, false);
	menuBar()->setItemChecked(ID_PYTHON, false);
	if (m_current == NULL) {
		return;
	}
	int lang = m_current->getLang();
	if (lang == Macro::LANG_LUA) {
		menuBar()->setItemChecked(ID_LUA, true);
	} else if (lang == Macro::LANG_PYTHON) {
		menuBar()->setItemChecked(ID_PYTHON, true);
	}
}
