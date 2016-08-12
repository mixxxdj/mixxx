#ifndef LAYOUTSFILEHANDLER_H
#define LAYOUTSFILEHANDLER_H


#include <QFile>
#include <QDir>

typedef QList<QStringList> LayoutNamesData;

class LayoutsFileHandler {
public:
    LayoutsFileHandler();
    virtual ~LayoutsFileHandler();

    void open(const QString layoutsPath);
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

    void compileLayoutsFile(const QString cppPath);
};



#endif // LAYOUTSFILEHANDLER_H
