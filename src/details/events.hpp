//
// Created by niko on 09.06.2021.
//

#ifndef REDIS_ASYNC_EVENTS_HPP
#define REDIS_ASYNC_EVENTS_HPP

namespace redis_async {
    namespace details {
        namespace events {

            struct execute {};
            struct terminate {};
            struct complete {};
            struct ready_for_query {};

        } // namespace events
    } // namespace details
} // namespace redis_async

#endif // REDIS_ASYNC_EVENTS_HPP
