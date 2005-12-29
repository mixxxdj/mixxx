#ifndef SCRIPT_SCRIPTSTUDIO_H
#define SCRIPT_SCRIPTSTUDIO_H

#include <qmainwindow.h>
#include <qtextedit.h>
#include "macrolist.h"
#include "scriptengine.h"
#include "scriptrecorder.h"

class MacroList;

class ScriptStudio : public QMainWindow {
	Q_OBJECT
	
	public:
		ScriptStudio(ScriptEngine* model, QWidget* parent = 0, const char* name = "Script Studio");
		~ScriptStudio();

	public slots:
		void newScript();
		void changeScript();
		void editScript();
		void recordScript();
		void stopRecord();
		void playMacro();
		void deleteMacro();

		void showStudio();
	private:
		ScriptEngine* m_model;
		QTextEdit* m_edit;
		MacroList* m_mlist;
		Macro* m_current;

		ScriptRecorder m_rec;
};
#endif
