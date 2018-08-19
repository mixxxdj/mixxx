#ifndef CONTROLLERENGINEEXCEPTIONS_H
#define CONTROLLERENGINEEXCEPTIONS_H

#include <exception>

#include <QString>

class NullEngineException: public std::exception {};

class EvaluationException: public std::exception {
  public:
	EvaluationException(QString errorMessage, QString line,
			QString backtrace, QString filename)
  	  : errorMessage(errorMessage), line(line), backtrace(backtrace), filename(filename) {};

    QString errorMessage;
    QString line;
    QString backtrace;
    QString filename;
};

#endif
