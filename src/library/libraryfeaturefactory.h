/*
 * LibraryFeatureFactory.h
 *
 *  Created on: Jun 25, 2016
 *      Author: joan
 */

#ifndef SRC_LIBRARY_LIBRARYFEATUREFACTORY_H_
#define SRC_LIBRARY_LIBRARYFEATUREFACTORY_H_
#include <QString>
#include <QHash>

class LibraryFeature;

/**
 * To use this we trust in the compiler initialization of static 
 */
#define REGISTER_F(featureName) \
  private: \
    static LibraryFeatureCreator<featureName> creator;


class Creator {
  public:
    virtual ~Creator() = 0;
    virtual LibraryFeature* create() = 0;
};

template<class T>
class LibraryFeatureCreator : Creator {
    LibraryFeature* create() { 
        return new T; 
    }
};

class LibraryFeatureFactory {
public:
    LibraryFeatureFactory();
    ~LibraryFeatureFactory();
    
    static void registerF(const QString& name, Creator* pCreator);
    
    static QHash<QString, Creator*> getTable();
    
    static LibraryFeature* create(const QString& name);
};

#endif /* SRC_LIBRARY_LIBRARYFEATUREFACTORY_H_ */
