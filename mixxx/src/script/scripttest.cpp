#include "scripttest.h"

ScriptTest::ScriptTest(ScriptEngine *peng, QWidget *parent, const char* name)
	: QVBox(parent, name) {
	m_parent = peng;

	m_rec = peng->getRecorder();
	
	m_text = new QLineEdit(this);
	m_exec = new QPushButton("Execute", this);
	m_record = new QPushButton("Record", this);
	m_stop = new QPushButton("Stop", this);
	m_stop->setEnabled(false);
	m_dump = new QPushButton("Dump", this);
	m_dump->setEnabled(false);
	m_play = new QPushButton("Play", this);
	m_play->setEnabled(false);
	m_clear = new QPushButton("Clear", this);
	m_clear->setEnabled(false);
	
	connect(m_exec, SIGNAL(clicked()), this, SLOT(executeText()));
	connect(m_record, SIGNAL(clicked()), this, SLOT(record()));
	connect(m_stop, SIGNAL(clicked()), this, SLOT(stop()));
	connect(m_dump, SIGNAL(clicked()), this, SLOT(dump()));
	connect(m_play, SIGNAL(clicked()), this, SLOT(play()));
	connect(m_clear, SIGNAL(clicked()), this, SLOT(clear()));
	
	show();
}

ScriptTest::~ScriptTest() {
}

void ScriptTest::executeText() {
	QString text = m_text->text();
	m_parent->executeScript(text);
}

void ScriptTest::record() {
	m_rec->startRecord();
	m_record->setEnabled(false);
	m_stop->setEnabled(true);
}

void ScriptTest::dump() {
	qDebug(*m_rec->getMacro());
}

void ScriptTest::play() {
	m_parent->executeScript(*m_rec->getMacro());
}

void ScriptTest::stop() {
	m_rec->stopRecord();
	m_stop->setEnabled(false);
	m_play->setEnabled(true);
	m_dump->setEnabled(true);
	m_clear->setEnabled(true);
}

void ScriptTest::clear() {
	m_rec->reset();
	m_play->setEnabled(false);
	m_dump->setEnabled(false);
	m_record->setEnabled(true);
	m_clear->setEnabled(false);
}
