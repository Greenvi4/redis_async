//
// Created by niko on 09.06.2021.
//

#ifndef REDIS_ASYNC_EVENTS_HPP
#define REDIS_ASYNC_EVENTS_HPP

#include <redis_async/common.hpp>
#include <redis_async/rd_types.hpp>

namespace redis_async {
    namespace details {

        namespace events {

            struct execute {
                using Buffer = std::string;

                Buffer buff;
                query_result_callback result;
                error_callback error;
            };
            struct recv {
                result_t res;
            };
            struct terminate {};
            struct complete {};

        } // namespace events

    } // namespace details
} // namespace redis_async

#endif // REDIS_ASYNC_EVENTS_HPP
