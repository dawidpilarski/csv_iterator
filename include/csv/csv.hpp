/*
 * Copyright 2020 dawid.pilarski@panicsoftware.com
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <string>
#include <istream>
#include <string_view>
#include <array>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <fstream>

namespace csv {

struct csv_error : std::runtime_error{
  using std::runtime_error::runtime_error;
};

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
  constexpr bool file_without_commas = result_array_size == 1;

  if constexpr (first_iteration && file_without_commas){
    auto* data = ::std::addressof(*begin);
    auto* data_end = ::std::addressof(*end);
    result[current_result_idx] = ::std::string_view(data, data_end - data);
    return result;
  }
  else if constexpr (first_iteration){
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
  static_assert(rows_ >= 1, "csv_iterators needs to operate on stream, that has at least one column");
 public:
  static constexpr std::size_t rows = rows_;
  using iterator_category = ::std::input_iterator_tag;
  using value_type = ::std::array<::std::string_view, rows>;
  using pointer = const ::std::array<::std::string_view, rows>*;
  using reference = const ::std::array<::std::string_view, rows>&;
  using difference_type = std::ptrdiff_t;

  csv_iterator() noexcept : stream_(nullptr) {}

  explicit csv_iterator(std::istream& stream, char delimiter = ',') :
      delimiter_(delimiter),
      stream_(&stream) {
    operator++();
  }

  csv_iterator(const csv_iterator& rhs) :
  delimiter_(rhs.delimiter_),
  stream_(rhs.stream_),
  result_(rhs.result_){
    parse_line(); // todo optimize
  }

  csv_iterator(csv_iterator&& rhs) noexcept :
  delimiter_(rhs.delimiter_),
  stream_(rhs.stream_),
  result_(std::move(rhs.result_))
  {
    parse_line(); // todo optimize
  }

  csv_iterator& operator=(const csv_iterator& rhs){
    delimiter_ = rhs.delimiter_;
    stream_ = rhs.stream_;
    result_ = rhs.result_;
    parse_line(); // update string_views in cached result

    return *this;
  }

  csv_iterator& operator=(csv_iterator&& rhs){
    delimiter_ = rhs.delimiter_;
    stream_ = rhs.stream_;
    result_ = std::move(rhs.result_);
    parse_line(); // update string_views in cached result

    return *this;
  }

  ~csv_iterator() = default;

  reference operator*() const {
    return result_.result;
  }

  pointer operator->() const {
    return &result_.result;
  }

  csv_iterator& operator++() {
    if(!::std::getline(*stream_, result_.line_)){
      std::cerr << std::boolalpha << stream_->bad() << stream_->eof() << stream_->fail();
      stream_ = nullptr;
      return *this;
    }

    parse_line();
    return *this;
  }

  csv_iterator operator++(int) {
    csv_iterator previous = *this;
    ++(*this);
    return std::move(previous);
  }

 private:

  void parse_line(){
    auto predicate = [this](char letter){return letter == delimiter_;};
    if constexpr (check_correctness) {
      auto comma_positions = details::find_all(result_.line_.begin(), result_.line_.end(), predicate);
      if(comma_positions.size() != rows - 1){
        throw csv_error("csv file contains wrong number of rows.");
      }

      auto* comma_positions_array = comma_positions.data();
      result_.result = details::create_result<rows>(result_.line_.begin(), result_.line_.end(), comma_positions_array);
    } else {
      auto comma_positions = details::find_n<rows-1>(result_.line_.begin(),
                                                     result_.line_.end(),
                                                     predicate);
      result_.result = details::create_result<rows>(result_.line_.begin(), result_.line_.end(), comma_positions);
    }
  }

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