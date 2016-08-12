#ifndef LAYOUTSFILEHANDLER_H
#define LAYOUTSFILEHANDLER_H


#include <QFile>
#include <QDir>

class LayoutsFileHandler {
public:
    LayoutsFileHandler();
    virtual ~LayoutsFileHandler();

    void open(QDir& layoutsPath);
    void save(QFile& file);
};


#endif // LAYOUTSFILEHANDLER_H
