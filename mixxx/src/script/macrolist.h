#ifndef SCRIPT_MACROLIST_H
#define SCRIPT_MACROLIST_H

#include "scriptengine.h"

class ScriptEngine;

class MacroList : public Q3ListView {
	Q_OBJECT

	public:
		MacroList(ScriptEngine* model, QWidget* parent = 0, const char* name = "Macro List");
		~MacroList();
		void repaint();

	private:
		ScriptEngine* m_model;
};
#endif
