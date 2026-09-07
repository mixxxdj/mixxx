#pragma once

#include <QHash>
#include <iterator>

template<typename C, typename I>
typename C::const_iterator make_const_iterator(const C&, const I& it) {
    return static_cast<typename C::const_iterator>(it);
}

/// This template returns a non-const iterator from a container pointing to the
/// same element as the given const iterator.
/// This can be used in Qt5 wrappers implementing the Qt6 interface
/// where some iterators have been made const.
template<typename C, typename I>
typename C::iterator make_iterator(C* pContainer, I it) {
    return std::next(pContainer->begin(), std::distance(pContainer->cbegin(), it));
}

// Specialization for QHash
template<class Key, class T>
typename QHash<Key, T>::iterator make_iterator(
        QHash<Key, T>* pContainer, typename QHash<Key, T>::const_iterator it) {
    return pContainer->find(it.key());
}

template<typename C, typename I>
typename C::const_iterator constErase(C* pContainer, I begin, I end) {
    return make_const_iterator(*pContainer, pContainer->erase(begin, end));
}

template<typename C, typename I>
typename C::iterator erase(C* pContainer, I begin, I end) {
    return pContainer->erase(make_const_iterator(*pContainer, begin),
            make_const_iterator(*pContainer, end));
}

template<typename C, typename I>
typename C::const_iterator constErase(C* pContainer, I it) {
    return make_const_iterator(*pContainer, pContainer->erase(it));
}

template<typename C, typename I>
typename C::iterator erase(C* pContainer, I it) {
    return pContainer->erase(make_const_iterator(*pContainer, it));
}

template<typename C, typename I, class T>
typename C::const_iterator constInsert(C* pContainer, I before, T t) {
    return make_const_iterator(*pContainer, pContainer->insert(before, std::move(t)));
}

template<typename C, typename I, class T>
typename C::iterator insert(C* pContainer, I before, T t) {
    return pContainer->insert(make_const_iterator(*pContainer, before), std::move(t));
}
