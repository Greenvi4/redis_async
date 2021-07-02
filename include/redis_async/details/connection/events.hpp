//
// Created by niko on 09.06.2021.
//

#ifndef REDIS_ASYNC_EVENTS_HPP
#define REDIS_ASYNC_EVENTS_HPP

#include <redis_async/common.hpp>
#include <redis_async/rd_types.hpp>
#include <redis_async/details/protocol/command.hpp>

namespace redis_async {
    namespace details {

        namespace events {

            struct execute {
                command_wrapper_t cmd;
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
