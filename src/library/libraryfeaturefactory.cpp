/*
 * LibraryFeatureFactory.cpp
 *
 *  Created on: Jun 25, 2016
 *      Author: joan
 */

#include <library/libraryfeature.h>
#include <library/libraryfeaturefactory.h>

LibraryFeatureFactory::LibraryFeatureFactory() {
    // TODO Auto-generated constructor stub
    
}

LibraryFeatureFactory::~LibraryFeatureFactory() {
    // TODO Auto-generated destructor stub
}

void LibraryFeatureFactory::registerF(const QString& name,
        Creator* pCreator) {
    getTable()[name] = pCreator;
}

QHash<QString, Creator*> LibraryFeatureFactory::getTable() {
    static QHash<QString, Creator*> table;
    return table;
}
