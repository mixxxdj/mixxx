#ifndef SCRIPT_SCRIPTTEST_H
#define SCRIPT_SCRIPTTEST_H

#include "scriptengine.h"
#include "scriptrecorder.h"
#include <qwidget.h>
#include <q3vbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>

class ScriptTest : public Q3VBox {
	Q_OBJECT
	
	public:
		ScriptTest(ScriptEngine *peng, QWidget *parent=0, const char* name="Mixxx Script Test");
		~ScriptTest();
		
		ScriptEngine *m_parent;
		
	public slots:
		void executeText();
		void record();
		void dump();
		void play();
		void stop();
		void clear();
	
	private:
		QLineEdit *m_text;
		QPushButton *m_exec;
		QPushButton *m_record;
		QPushButton *m_stop;
		QPushButton *m_dump;
		QPushButton *m_play;
		QPushButton *m_clear;
		
		ScriptRecorder *m_rec;
};

#endif
