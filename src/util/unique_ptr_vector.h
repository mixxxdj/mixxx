#pragma once

#include <memory>
#include <vector>

// This wrapper class can be used as a workaround for bug in QT 6.4
// https://bugreports.qt.io/browse/QTBUG-109745 that prevents to use
// std::vector<std::unique_ptr<T>> in QVarLengthArray
// The only purpose of this class is to delete the copy constructor from
// std::vector<std::unique_ptr<T>> and make std::is_copy_constructible_v<> false
// std::vector<std::unique_ptr<T>> cannot be copied even if it is empty due to a
// static_assert in stl_uninitialized.h
template<typename T>
class unique_ptr_vector : public std::vector<std::unique_ptr<T>> {
    using std::vector<std::unique_ptr<T>>::vector;

  public:
    unique_ptr_vector(const unique_ptr_vector&) = delete;
    unique_ptr_vector(unique_ptr_vector&&) noexcept = default;
    unique_ptr_vector& operator=(const unique_ptr_vector&);
};
