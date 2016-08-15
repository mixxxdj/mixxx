#ifndef LAYOUTSFILEHANDLER_H
#define LAYOUTSFILEHANDLER_H


#include <QFile>
#include <QDir>
#include "layout.h"

typedef QList<QStringList> LayoutNamesData;
typedef KeyboardLayoutPointer (*GetLayout_t)(std::string layoutName);

class LayoutsFileHandler {
public:
    LayoutsFileHandler();
    virtual ~LayoutsFileHandler();

    void open(QString layoutsPath, QList<Layout> &layouts);
    void save(QFile& file);

private:
    LayoutNamesData getLayoutNames(QFile &cppFile);

    //// Appends function to layouts.h to retrieve layouts. Function could look like this:
    ////
    //// extern "C" KeyboardLayoutPointer getLayout(std::string layoutName) {
    ////    if (layoutName == "en_US") return en_US;
    ////    if (layoutName == "es_ES") return es_ES;
    ////    if (layoutName == "fr_FR") return fr_FR;
    ////    else {
    ////        return nullptr;
    ////    }
    //// }
    void appendGetLayoutsFunction(QFile &cppFile, const LayoutNamesData &layoutNames);

    void compileLayoutsFile(const QString cppPath, GetLayout_t &pFunction, void *&handle);
};



#endif // LAYOUTSFILEHANDLER_H
