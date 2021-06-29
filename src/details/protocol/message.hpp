//
// Created by niko on 29.06.2021.
//

#ifndef REDIS_ASYNC_MESSAGE_HPP
#define REDIS_ASYNC_MESSAGE_HPP

#include <boost/noncopyable.hpp>
#include <vector>

#include <details/protocol/command.hpp>

namespace redis_async {
    namespace details {

        class message : boost::noncopyable {
        public:
            /** Buffer type for the message */
            typedef std::vector<char> buffer_type;

            /** Input iterator for the message buffer */
            typedef buffer_type::const_iterator const_iterator;
            /** Output iterator for the message buffer */
            typedef std::back_insert_iterator<buffer_type> output_iterator;

            /** A range of iterators for input */
            typedef std::pair<const_iterator, const_iterator> const_range;

            /** Length type for the message */
            typedef std::size_t size_type;

        public:
            /**
             * Construct message for reading from the stream
             */
            message(const command_wrapper_t &command);

            /**
             * Message is move-only
             */
            message(message &&other) noexcept;

            size_t size() const;

            /**
             * A pair of iterators for constructing buffer for writing into the stream
             */
            const_range buffer() const;

        private:
            mutable buffer_type payload;
        };

    } // namespace details
} // namespace redis_async

#endif // REDIS_ASYNC_MESSAGE_HPP
