// Misc. container helper functions.

#ifndef CONTAINER_H
#define CONTAINER_H

#include <QMap>

// Helper function to delete the values of a QMap if they are pointers.
template <class K, class V>
void deleteMapValues(QMap<K,V>* pMap) {
    if (pMap == NULL) {
        return;
    }
    QMutableMapIterator<K, V> it(*pMap);
    while (it.hasNext()) {
        it.next();
        V value = it.value();
        it.remove();
        delete value;
    }
}

#endif /* CONTAINER_H */
