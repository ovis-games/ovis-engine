#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace ovis {

template <typename T>
class sparse_vector {
 public:
  // The definitions according to the standard containers
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;

  template <bool is_const>
  class basic_iterator;
  using iterator = basic_iterator<false>;
  using const_iterator = basic_iterator<true>;

  sparse_vector() : data_(nullptr), free_positions_(nullptr), size_(0), capacity_(0) {}

  ~sparse_vector() {
    for (size_type i = 0; i < capacity_; ++i) {
      if (!free_positions_[i]) {
        reinterpret_cast<value_type&>(data_[i]).~value_type();
      }
    }
    delete[] data_;
    delete[] free_positions_;
  }

  // These can be implemented, but arent yet.
  sparse_vector(const sparse_vector& other) = delete;
  sparse_vector(sparse_vector&& other) = delete;
  sparse_vector& operator=(const sparse_vector& other) = delete;
  sparse_vector& operator==(sparse_vector&& other) = delete;

  // Lookup
  inline reference at(size_type index) {
    if (!contains(index)) {
      throw std::out_of_range("index is out of range");
    } else {
      return reinterpret_cast<reference>(data_[index]);
    }
  }
  inline const_reference at(size_type index) const {
    if (!contains(index)) {
      throw std::out_of_range("index is out of range");
    } else {
      return reinterpret_cast<const_reference>(data_[index]);
    }
  }

  inline reference operator[](size_type index) {
    assert(contains(index));
    return reinterpret_cast<reference>(data_[index]);
  }
  inline const_reference operator[](size_type index) const {
    assert(contains(index));
    return reinterpret_cast<const_reference>(data_[index]);
  }

  inline bool contains(size_type index) const { return index < capacity_ && !free_positions_[index]; }

  // Iterators
  inline constexpr iterator begin() noexcept {
    const size_type first_occupied_index = find_first_occupied_index();
    return iterator(data_ + first_occupied_index, free_positions_ + first_occupied_index);
  }
  inline constexpr const_iterator begin() const noexcept {
    const size_type first_occupied_index = find_first_occupied_index();
    return const_iterator(data_ + first_occupied_index, free_positions_ + first_occupied_index);
  }
  inline constexpr const_iterator cbegin() const noexcept {
    const size_type first_occupied_index = find_first_occupied_index();
    return const_iterator(data_ + first_occupied_index, free_positions_ + first_occupied_index);
  }
  inline constexpr iterator end() noexcept { return iterator(data_ + capacity_, free_positions_ + capacity_); }
  inline constexpr const_iterator end() const noexcept {
    return const_iterator(data_ + capacity_, free_positions_ + capacity_);
  }
  inline constexpr const_iterator cend() const noexcept {
    return const_iterator(data_ + capacity_, free_positions_ + capacity_);
  }

  // Capacity
  inline bool empty() const { return size_ == 0; }
  inline size_type size() const { return size_; }
  inline size_type capacity() const { return capacity_; }
  inline constexpr void reserve(size_type new_cap) {
    if (new_cap > capacity_) {
      value_storage* data = new value_storage[new_cap];
      bool* free_positions = new bool[new_cap + 1];

      for (size_type i = 0; i < capacity_; ++i) {
        if (!free_positions_[i]) {
          // TODO: nothing good happens if this throws...
          value_type* old_value = reinterpret_cast<value_type*>(data_ + i);
          new (data + i) value_type(std::move(*old_value));
          old_value->~value_type();
          free_positions[i] = false;
        } else {
          free_positions[i] = true;
        }
      }
      for (size_type i = capacity_; i < new_cap; ++i) {
        free_positions[i] = true;
      }
      free_positions[new_cap] = false;

      std::swap(data_, data);
      std::swap(free_positions_, free_positions);
      delete[] data;
      delete[] free_positions;
      capacity_ = new_cap;
    }
  }

  // Modifiers
  iterator insert(const value_type& value) {
    size_type insert_index;
    if (size_ < capacity_) {
      auto free_index = find_free_index();
      assert(free_index.has_value());
      insert_index = *free_index;
    } else {
      insert_index = capacity_;
      enlarge();
      assert(insert_index < capacity_);
    }
    new (data_ + insert_index) value_type(value);
    free_positions_[insert_index] = false;
    ++size_;
    return iterator(data_ + insert_index, free_positions_ + insert_index);
  }

  inline constexpr iterator erase(iterator it) {
    it->~value_type();
    *it.is_free_ = true;
    --size_;
    return it++;
  }

 private:
  using value_storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
  value_storage* data_;
  bool* free_positions_;
  size_type size_;
  size_type capacity_;

  std::optional<size_type> find_free_index() const {
    for (size_type i = 0; i < capacity_; ++i) {
      if (free_positions_[i]) {
        return i;
      }
    }
    return {};
  }

  void enlarge() {
    if (capacity_ < 8) {
      reserve(8);
    } else {
      reserve(capacity_ * 3 / 2);
    }
  }

  size_type find_first_occupied_index() const {
    if (size_ == 0) {
      return capacity_;
    } else {
      size_type index = 0;
      while (free_positions_[index]) {
        ++index;
      }
      return index;
    }
  }
};

template <typename T>
template <bool is_const>
class sparse_vector<T>::basic_iterator {
 public:
  friend class sparse_vector<T>;
  using value_type = std::conditional_t<is_const, T, const T>;

  basic_iterator(const iterator&) = default;
  ~basic_iterator() = default;

  inline basic_iterator(sparse_vector<T>::value_storage* value, bool* is_free) : value_(value), is_free_(is_free) {}

  basic_iterator& operator=(const basic_iterator&) = default;

  inline bool operator==(const basic_iterator& rhs) const { return value_ == rhs.value_; }

  inline bool operator!=(const basic_iterator& rhs) const { return value_ != rhs.value_; }

  inline value_type& operator*() const { return *reinterpret_cast<value_type*>(value_); }

  inline value_type* operator->() const { return reinterpret_cast<value_type*>(value_); }

  inline basic_iterator& operator++() {
    do {
      ++value_;
      ++is_free_;
    } while (*is_free_);
    return *this;
  }

  inline basic_iterator operator++(int) {
    basic_iterator result(*this);
    do {
      ++value_;
      ++is_free_;
    } while (*is_free_);
    return result;
  }

 private:
  sparse_vector<T>::value_storage* value_;
  bool* is_free_;
};

}  // namespace ovis
