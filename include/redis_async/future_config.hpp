//
// Created by niko on 24.05.2021.
//

#ifndef REDIS_ASYNC_FUTURE_CONFIG_HPP
#define REDIS_ASYNC_FUTURE_CONFIG_HPP

#ifdef REDIS_ASYNC_WITH_BOOST_FIBERS
    #include <boost/fiber/fiber.hpp>
    #include <boost/fiber/future.hpp>
#else
    #include <future>
#endif

namespace redis_async {

#ifdef REDIS_ASYNC_WITH_BOOST_FIBERS
    template <typename _Res>
    using promise = ::boost::fibers::promise<_Res>;
    using fiber = ::boost::fibers::fibert
#else
    template <typename TRes>
    using promise = ::std::promise<TRes>;
#endif

} // namespace redis_async

#endif // REDIS_ASYNC_FUTURE_CONFIG_HPP
