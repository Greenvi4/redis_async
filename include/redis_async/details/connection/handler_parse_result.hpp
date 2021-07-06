//
// Created by niko on 06.07.2021.
//

#ifndef REDIS_ASYNC_HANDLER_PARSE_RESULT_HPP
#define REDIS_ASYNC_HANDLER_PARSE_RESULT_HPP

#include <redis_async/details/connection/events.hpp>
#include <redis_async/details/protocol/parser_types.hpp>
#include <redis_async/error.hpp>

namespace redis_async {
    namespace details {

        template <typename FSM>
        struct handler_parse_result_t : public boost::static_visitor<std::size_t> {

            explicit handler_parse_result_t(FSM &fsm)
                : m_fsm(fsm) {
            }

            std::size_t operator()(protocol_error_t &err) const {
                m_fsm.process_event(error::query_error{err.code.message()});
                return 0;
            }

            std::size_t operator()(error_t &err) const {
                m_fsm.process_event(error::query_error{err.str});
                return err.consumed;
            }

            std::size_t operator()(positive_parse_result_t &res) const {
                m_fsm.process_event(events::recv{std::move(res.result)});
                return res.consumed;
            }

        private:
            FSM &m_fsm;
        };

    } // namespace details
} // namespace redis_async

#endif // REDIS_ASYNC_HANDLER_PARSE_RESULT_HPP
