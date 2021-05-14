#pragma once

#include <QDomNode>
#include <QDomElement>
#include <QDomDocument>

class XmlParse {
  public:
    // Searches for and returns a node named sNode in the children of
    // nodeHeader. Returns a null QDomNode if one is not found.
    static QDomNode selectNode(const QDomNode& nodeHeader,
                               const QString& sNode);

    // Searches for and returns an element named sNode in the children of
    // nodeHeader. Returns a null QDomElement if one is not found.
    static QDomElement selectElement(const QDomNode& nodeHeader,
                                     const QString& sNode);

    // Searches for an element named sNode in the children of nodeHeader and
    // parses the text value of its children as an integer. Returns 0 if sNode
    // is not found in nodeHeader's children.
    static int selectNodeInt(const QDomNode& nodeHeader,
                             const QString& sNode, bool* ok = 0);

    // Searches for an element named sNode in the children of nodeHeader and
    // parses the text value of its children as a float. Returns 0.0f if sNode
    // is not found in nodeHeader's children.
    static float selectNodeFloat(const QDomNode& nodeHeader,
                                 const QString& sNode, bool* ok = 0);

    // Searches for an element named sNode in the children of nodeHeader and
    // parses the text value of its children as a double. Returns 0.0 if sNode
    // is not found in nodeHeader's children.
    static double selectNodeDouble(const QDomNode& nodeHeader,
                                   const QString& sNode, bool* ok = 0);

    // Searches for an element named sNode in the children of nodeHeader and
    // returns the text value of its children. Returns the empty string if sNode
    // is not found in nodeHeader's children.
    static QString selectNodeQString(const QDomNode& nodeHeader,
                                     const QString& sNode);

    // Open an XML file.
    static QDomElement openXMLFile(const QString& path, const QString& name);

    // Add an element named elementName to the provided header element with a
    // child text node with the value textValue. Returns the created element.
    static QDomElement addElement(QDomDocument& document, QDomElement& header,
                                  const QString& elementName, const QString& textValue);

  private:
    XmlParse() {}
    ~XmlParse() {}
};
