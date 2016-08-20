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

    // Read cpp file with given path and fill given layouts with Layout objects
    void open(QString& layoutsPath, QList<Layout>& layouts);

    // Generate code for given layouts and overwrite given file with that code.
    void save(QFile& f, QList<Layout>& layouts);

  private:
    // Iterate through lines in cpp file and find out with string comparisons
    // the names of the layouts that are in the file (both variable names as
    // names as placed in comments)
    //
    // // Spanish (Spain)                   <-- Layout name   (LayoutNamesData[1])
    // static const KbdKeyChar[48][2] es_ES <-- variable name (LayoutNamesData[0])
    //
    // NOTE: If no comment is found, layout name will be the same as the
    //       variable name
    LayoutNamesData getLayoutNames(QFile& cppFile);

    // Prepends definitions to layouts.h in order for this tool to be able to compile
    // the layouts without errors
    void prependDefs(QFile& cppFile);

    // Appends function to layouts.h to access its layouts. When forInternUse is
    // true it prepends the function with "extern 'C'".
    void appendGetLayoutsFunction(QFile& cppFile,
                                  const LayoutNamesData& layoutNames,
                                  bool forInternUse);

    // Remove sections surrounded by kSkipCommentHead and kSkipCommentTail comments
    void removeSkipParts(QFile& file);

    // Compiles cpp layouts file and points given GetLayout_t function pointer to a function
    // that can be used to access the layouts in the file. When done using this function, close
    // the file using dlclose(handle)
    void compileLayoutsFile(const QString& cppPath, GetLayout_t& pFunction, void*& handle);

    // Generate header file and overwrite given file
    void createHeaderFile(const QString& path);

    // Overwrite given file with QStringList where each QString represents one line.
    void overwriteFile(QFile& file, const QStringList& lines);

    // Append given QStringList to given file
    void appendToFile(QFile& file, const QStringList& lines);

    // Prepend given QStringList to given file
    void prependToFile(QFile& file, const QStringList& lines);
};

#endif // LAYOUTSFILEHANDLER_H
