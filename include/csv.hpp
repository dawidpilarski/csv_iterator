

#pragma once

#include <iostream>
#include <string>
#include <istream>
#include <string_view>
#include <array>
#include <vector>
#include <algorithm>
#include <fstream>

namespace csv {

namespace details {

template<typename Iter, typename Predicate>
auto find_all(Iter begin, const Iter end, Predicate pred) {
  ::std::vector<typename ::std::iterator_traits<Iter>::pointer> elements;
  auto& current = begin;
  while (current != end) {
    auto found_element = ::std::find_if(current, end, pred);
    if (found_element != end) {
      elements.push_back(std::addressof(*found_element));
      ++found_element;
    }
    current = found_element;
  }

  return elements;
}

template<std::size_t N, typename Iter, typename Predicate, typename Array = ::std::array<typename ::std::iterator_traits<Iter>::pointer, N>>
Array find_n(Iter begin, Iter end, Predicate pred, Array&& arr = Array{}) {
  if constexpr (N == 0) return arr;
  else {
    auto found_element = ::std::find_if(begin, end, pred);
    arr[arr.size() - N] = std::addressof(*found_element);
    return find_n<N-1>(++found_element, end, pred, arr);
  }
}

template <typename T>
struct size;

template <typename T, std::size_t N>
struct size<std::array<T, N>>{
  static constexpr std::size_t value = N;
};

template <std::size_t N, typename Iter, typename Array, typename ArrResult = ::std::array<::std::string_view , N>>
ArrResult create_result(Iter begin, Iter end, Array&& comma_array, ArrResult&& result= ArrResult{}){
  constexpr std::size_t result_array_size = size<::std::remove_reference_t<ArrResult>>::value;
  constexpr std::size_t current_result_idx = result_array_size - N;
  constexpr bool first_iteration = (current_result_idx == 0);
  constexpr bool last_iteration = (current_result_idx == result_array_size-1);

  if constexpr (first_iteration){
    auto* data = ::std::addressof(*begin);
    result[current_result_idx] = ::std::string_view(data, comma_array[0] - data);
    return create_result<N-1>(begin, end, comma_array, result);
  }
  else if constexpr (last_iteration){
    constexpr std::size_t current_comma_idx = current_result_idx -1;
    auto* pointer_past_comma = comma_array[current_comma_idx]+1;
    result[current_result_idx] = ::std::string_view(pointer_past_comma, std::addressof(*end) - pointer_past_comma);
    return result;
  }
  else {
    constexpr std::size_t current_comma_idx = current_result_idx -1;
    auto* pointer_past_comma = comma_array[current_comma_idx]+1;
    result[current_result_idx] = ::std::string_view(pointer_past_comma, comma_array[current_comma_idx+1] - pointer_past_comma);
    return create_result<N-1>(begin, end, comma_array, result);
  }
}

}

template<std::size_t rows_, bool check_correctness = false>
class csv_iterator {
 public:
  static constexpr std::size_t rows = rows_;
  using iterator_category = ::std::input_iterator_tag;
  using value_type = ::std::array<::std::string_view, rows>;
  using pointer = const ::std::array<::std::string_view, rows>*;
  using reference = const ::std::array<::std::string_view, rows>&;

  csv_iterator() noexcept : stream_(nullptr) {}

  explicit csv_iterator(std::istream& stream, char delimiter = ',') :
      delimiter_(delimiter),
      stream_(&stream) {
    operator++();
  }

  csv_iterator(const csv_iterator&) = default;
  csv_iterator(csv_iterator&&) noexcept = default;
  csv_iterator& operator=(const csv_iterator&) = default;
  csv_iterator& operator=(csv_iterator&&) = default;
  ~csv_iterator() = default;

  reference operator*() const {
    return result_.result;
  }

  pointer operator->() const {
    return &result_.result;
  }

  csv_iterator& operator++() {
    if(!::std::getline(*stream_, result_.line_)){
      stream_ = nullptr;
      return *this;
    }

    auto predicate = [this](char letter){return letter == delimiter_;};
    if constexpr (check_correctness) {
      auto comma_positions = details::find_all(result_.line_.begin(), result_.line_.end(), predicate);
      if(comma_positions.size() != rows - 1){
        throw ::std::runtime_error("csv file contains wrong number of rows.");
      }

      auto* comma_positions_array = comma_positions.data();
      result_.result = details::create_result<rows>(result_.line_.begin(), result_.line_.end(), comma_positions_array);
    } else {
      auto comma_positions = details::find_n<rows-1>(result_.line_.begin(),
          result_.line_.end(),
          predicate);
      result_.result = details::create_result<rows>(result_.line_.begin(), result_.line_.end(), comma_positions);
    }

    return *this;
  }

  csv_iterator operator++(int) {
    csv_iterator previous = *this;
    ++(*this);
    return std::move(previous);
  }

 private:
  struct cached_result {
    ::std::string line_;
    ::std::array<::std::string_view, rows> result;
  };

  friend bool operator==(const csv_iterator& lhs, const csv_iterator& rhs) {
    return lhs.stream_ == rhs.stream_;
  }

  friend bool operator!=(const csv_iterator& lhs, const csv_iterator& rhs) {
    return !(lhs == rhs);
  }

  friend csv_iterator& begin(csv_iterator& iterator){
    return iterator;
  }

  friend csv_iterator begin(const csv_iterator& iterator){
    return iterator;
  }

  friend csv_iterator end(const csv_iterator&){
    return csv_iterator{};
  }

  char delimiter_;
  ::std::istream *stream_;
  cached_result result_;
};

}