#pragma once

#include "roo_backport/string_view.h"

namespace roo_io {

template <typename CharT, typename Traits = std::char_traits<CharT>>
using basic_string_view = roo::basic_string_view<CharT, Traits>;

using string_view = basic_string_view<char>;

};  // namespace roo_io
