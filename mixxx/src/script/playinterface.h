#ifndef SCRIPT_PLAYINTERFACE_H
#define SCRIPT_PLAYINTERFACE_H

#include "scriptcontrolqueue.h"
#include <QLinkedList>
#include "qdatetime.h"

class PlayInterface {
	public:
		PlayInterface(ScriptControlQueue* q);
		~PlayInterface();
		void setProcess(int);
		void clearProcess();
		void doStartFade(const char* group, const char* name, \
				int interp);	
		// Functions for scripts to call below here
		void stop(int channel);
		void play(int channel);
		void setFader(double fade);
		void test();

		void setTag(int tag);
		void clearTag();
	
		void kill();
		void killTag(int tag);
		
		double getFader();
		double getValue(const char* group, const char* name);
		
		void startFadeCrossfader();
		void startList(const char* group, const char* name);
		void startFade(const char* group, const char* name);
		void point(int time, double value);
		void fadePoint(int time, double value);
		void endFade();
		void endList();

		void playChannel1(int time, char* path);
		void playChannel2(int time, char* path);


	private:
		ScriptControlQueue* m_q;
		
		QDateTime m_time;
		QLinkedList<int>* m_times;
		QLinkedList<double>* m_values;
		
		const char* m_group;
		const char* m_name;
		int m_interp;
		int m_process;
		int m_tag;
};

#endif
