#pragma once

#include <cassert>
#include <cstddef>
#include <string>
#include <cstring>

namespace roo_io {

template <typename CharT, typename Traits = std::char_traits<CharT>>
class basic_string_view {
 public:
  using value_type = CharT;
  using pointer = const CharT *;
  using const_pointer = const CharT *;
  using reference = const CharT &;
  using const_reference = const CharT &;
  using iterator = const CharT *;
  using const_iterator = const CharT *;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = const_reverse_iterator;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  static constexpr size_type npos = size_type(-1);

  constexpr basic_string_view() noexcept : len_(0), str_(nullptr) {}

  constexpr basic_string_view(const basic_string_view &) noexcept = default;

  constexpr basic_string_view(const CharT *str, size_type len) noexcept
      : len_(len), str_(str) {}

  basic_string_view(const CharT *str) noexcept : len_(strlen(str)), str_(str) {}

  basic_string_view(const std::string &str) noexcept
      : len_(str.size()), str_(str.c_str()) {}

  basic_string_view &operator=(const basic_string_view &) noexcept = default;

  constexpr const_iterator begin() const noexcept { return str_; }

  constexpr const_iterator end() const noexcept { return str_ + len_; }

  constexpr const_iterator cbegin() const noexcept { return str_; }

  constexpr const_iterator cend() const noexcept { return str_ + len_; }

  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(begin());
  }

  constexpr size_type size() const noexcept { return len_; }

  constexpr bool empty() const noexcept { return len_ == 0; }

  constexpr const_reference operator[](size_type pos) const noexcept {
    return str_[pos];
  }

  constexpr const_reference at(size_type pos) const { return str_[pos]; }

  constexpr const_reference front() const noexcept { return str_[0]; }

  constexpr const_reference back() const noexcept { return str_[len_ - 1]; }

  constexpr const_pointer data() const noexcept { return str_; }

  void remove_prefix(size_type n) noexcept {
    assert(len_ >= n);
    str_ += n;
    len_ -= n;
  }

  void remove_suffix(size_type n) noexcept {
    assert(len_ >= n);
    len_ -= n;
  }

  void swap(basic_string_view &sv) noexcept {
    auto tmp = *this;
    *this = sv;
    sv = tmp;
  }

  basic_string_view substr(size_type pos, size_type n = npos) const {
    assert(pos <= len_);
    const size_type rlen = std::min(n, len_ - pos);
    return basic_string_view(str_ + pos, rlen);
  }

  int compare(basic_string_view str) const noexcept {
    const size_type rlen = std::min(len_, str.len_);
    int ret = strncmp(str_, str.str_, rlen);
    if (ret == 0) {
      ret = (len_ < str.len_) ? -1 : (len_ > str.len_) ? 1 : 0;
    }
    return ret;
  }

 private:
  size_t len_;
  const CharT *str_;
};

using string_view = basic_string_view<char>;

#if __cplusplus >= 202002L
using u8string_view = basic_string_view<char8_t>;
#endif

template <typename CharT>
inline bool operator==(basic_string_view<CharT> x,
                       basic_string_view<CharT> y) noexcept {
  return x.size() == y.size() && x.compare(y) == 0;
}

template <typename CharT>
inline bool operator!=(basic_string_view<CharT> x,
                       basic_string_view<CharT> y) noexcept {
  return !(x == y);
}

template <typename CharT>
inline bool operator<(basic_string_view<CharT> x,
                      basic_string_view<CharT> y) noexcept {
  return x.compare(y) < 0;
}

template <typename CharT>
inline bool operator>(basic_string_view<CharT> x,
                      basic_string_view<CharT> y) noexcept {
  return x.compare(y) > 0;
}

template <typename CharT>
inline bool operator<=(basic_string_view<CharT> x,
                       basic_string_view<CharT> y) noexcept {
  return x.compare(y) <= 0;
}

template <typename CharT>
inline bool operator>=(basic_string_view<CharT> x,
                       basic_string_view<CharT> y) noexcept {
  return x.compare(y) >= 0;
}

// template <typename CharT>
// inline std::ostream &operator<<(std::ostream &os, basic_string_view<CharT> v)
// {
//   os.write((const char *)v.data(), v.size());
//   return os;
// }

}  // namespace roo_io