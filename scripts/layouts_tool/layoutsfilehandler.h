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

    void open(QString &layoutsPath, QList<Layout> &layouts);
    void save(QFile &f, QList<Layout> &layouts);

    static const QString INDENT;
    static const QStringList HEADER_COMMENT;
    static const QString SKIP_HEAD;
    static const QString SKIP_TAIL;

    // Header file
    static const QStringList INCLUDE_GUARD_HEAD;
    static const QString KBDKEYCHAR_PROTOTYPE;
    static const QString KBDLAYOUTPOINTER_TYPEDEF;
    static const QString GETLAYOUT_FUNCTION_PROTOTYPE;
    static const QString INCLUDE_GUARD_TAIL;

    // Implementation file
    static const QString INCLUDE_STRING;
    static const QStringList KBDKEYCHAR_IMPLEMENTATION;

private:
    LayoutNamesData getLayoutNames(QFile &cppFile);

    //// Prepends definitions to layouts.h in order for this tool to be able to compile
    //// the layouts. It adds this:
    ////
    ////  struct KbdKeyChar {
    ////      char16_t character;
    ////      bool is_dead;
    ////  };
    ////
    ////  typedef const KbdKeyChar (*KeyboardLayoutPointer)[2];
    void prependDefs(QFile &cppFile);

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
    ////
    //// If forInternUse is false, it doesn't add 'extern "C" '
    void appendGetLayoutsFunction(QFile &cppFile,
                                  const LayoutNamesData &layoutNames,
                                  bool forInternUse);

    // Remove sections surrounded by SKIP_HEAD and SKIP_TAIL comments
    void removeSkipParts(QFile &file);

    void compileLayoutsFile(const QString cppPath, GetLayout_t &pFunction, void *&handle);

    void createHeaderFile(const QString path);
};

#endif // LAYOUTSFILEHANDLER_H
