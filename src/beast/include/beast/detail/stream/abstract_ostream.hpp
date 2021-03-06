//
// Copyright (c) 2013-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BEAST_DETAIL_STREAM_ABSTRACT_OSTREAM_HPP
#define BEAST_DETAIL_STREAM_ABSTRACT_OSTREAM_HPP

#include <beast/detail/stream/basic_abstract_ostream.hpp>

namespace beast {
namespace detail {

/** An abstract ostream for `char`. */
using abstract_ostream = basic_abstract_ostream <char>;

} // detail
} // beast

#endif
