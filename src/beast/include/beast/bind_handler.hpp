//
// Copyright (c) 2013-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BEAST_BIND_HANDLER_HPP
#define BEAST_BIND_HANDLER_HPP

#include <beast/detail/integer_sequence.hpp>
#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/handler_cont_helpers.hpp>
#include <boost/asio/detail/handler_invoke_helpers.hpp>
#include <functional>
#include <type_traits>
#include <utility>

namespace beast {

namespace detail {

/*  Nullary handler that calls Handler with bound arguments.

    The bound handler provides the same io_service execution
    guarantees as the original handler.
*/
template<class Handler, class... Args>
class bound_handler
{
private:
    using args_type = std::tuple<typename std::decay<Args>::type...>;

    Handler h_;
    args_type args_;

    template<class Tuple, std::size_t... S>
    static void invoke(Handler& h, Tuple& args,
        index_sequence<S...>)
    {
        h(std::get<S>(args)...);
    }

public:
    using result_type = void;

    template<class DeducedHandler>
    explicit
    bound_handler(DeducedHandler&& handler, Args&&... args)
        : h_(std::forward<DeducedHandler>(handler))
        , args_(std::forward<Args>(args)...)
    {
    }

    void
    operator()()
    {
        invoke(h_, args_,
            index_sequence_for<Args...> ());
    }

    void
    operator()() const
    {
        invoke(h_, args_,
            index_sequence_for<Args...> ());
    }

    friend
    void*
    asio_handler_allocate(
        std::size_t size, bound_handler* h)
    {
        return boost_asio_handler_alloc_helpers::
            allocate(size, h->h_);
    }

    friend
    void
    asio_handler_deallocate(
        void* p, std::size_t size, bound_handler* h)
    {
        boost_asio_handler_alloc_helpers::
            deallocate(p, size, h->h_);
    }

    friend
    bool
    asio_handler_is_continuation(bound_handler* h)
    {
        return boost_asio_handler_cont_helpers::
            is_continuation (h->h_);
    }

    template<class F>
    friend
    void
    asio_handler_invoke(F&& f, bound_handler* h)
    {
        boost_asio_handler_invoke_helpers::
            invoke(f, h->h_);
    }
};

} // detail

//------------------------------------------------------------------------------

/** Bind parameters to a completion handler, creating a wrapped handler.

    This function creates a new handler which  invoked with no parameters
    calls the original handler with the list of bound arguments. The passed
    handler and arguments are forwarded into the returned handler, which
    provides the same `io_service` execution guarantees as the original
    handler.

    Unlike `io_service::wrap`, the returned handler can be used in a
    subsequent call to `io_service::post` instead of `io_service::dispatch`,
    to ensure that the handler will not be invoked immediately by the
    calling function.

    Example:
    @code
    template<class AsyncReadStream, class ReadHandler>
    void
    do_cancel(AsyncReadStream& stream, ReadHandler&& handler)
    {
        stream.get_io_service().post(
            bind_handler(std::forward<ReadHandler>(handler),
                boost::asio::error::operation_aborted, 0));
    }
    @endcode

    @param handler The handler to wrap.

    @param args A list of arguments to bind to the handler. The
    arguments are forwarded into the returned object.
*/
template<class CompletionHandler, class... Args>
#if GENERATING_DOCS
implementation_defined
#else
detail::bound_handler<
    typename std::decay<CompletionHandler>::type, Args...>
#endif
bind_handler(CompletionHandler&& handler, Args&&... args)
{
    return detail::bound_handler<typename std::decay<
        CompletionHandler>::type, Args...>(std::forward<
            CompletionHandler>(handler),
                std::forward<Args>(args)...);
}

} // beast

namespace std {
template<class Handler, class... Args>
void bind(beast::detail::bound_handler<
    Handler, Args...>, ...) = delete;
} // std

#endif
