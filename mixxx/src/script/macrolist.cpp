#include "macrolist.h"
#include "macrolistitem.h"

MacroList::MacroList(ScriptEngine* model, QWidget* parent, 
		const char* name) : Q3ListView(parent, name) {
	m_model = model;
	addColumn("Macro Name");
	repaint();
}

MacroList::~MacroList() {
}

void MacroList::repaint() {
	clear();
	for (int i = 0; i < m_model->macroCount(); i++) {
		new MacroListItem(this, m_model->getMacro(i));
	}
}
