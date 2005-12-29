#ifndef SCRIPT_MACROLIST_H
#define SCRIPT_MACROLIST_H

#include <qlistview.h>

#include "scriptengine.h"

class ScriptEngine;

class MacroList : public QListView {
	Q_OBJECT

	public:
		MacroList(ScriptEngine* model, QWidget* parent = 0, const char* name = "Macro List");
		~MacroList();
		void repaint();

	private:
		ScriptEngine* m_model;
};
#endif
