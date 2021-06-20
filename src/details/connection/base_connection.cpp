//
// Created by niko on 09.06.2021.
//

#include "connection_fsm.hpp"
#include "transport.hpp"

namespace redis_async {
    namespace details {

        log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("redis_async.connection");

        template <typename TransportType>
        std::shared_ptr<concrete_connection<TransportType>>
        create_connection(const asio_config::io_service_ptr &svc, connection_options const &opts,
                          connection_callbacks const &callbacks) {
            using connection_type = concrete_connection<TransportType>;
            using concrete_connection_ptr = std::shared_ptr<connection_type>;

            concrete_connection_ptr conn(new connection_type(svc, callbacks));
            // needed to start the highest-level SM. This will call on_entry and mark the start
            // of the SM
            conn->start();
            conn->connect(opts);
            return conn;
        }

        basic_connection_ptr basic_connection::create(basic_connection::io_service_ptr svc,
                                                      const connection_options &opts,
                                                      const connection_callbacks &callbacks) {
            if (opts.schema == "tcp")
                return create_connection<tcp_transport>(svc, opts, callbacks);
            if (opts.schema == "unix")
                return create_connection<socket_transport>(svc, opts, callbacks);

            std::stringstream os;
            os << "Schema " << opts.schema << " is unsupported";
            throw error::connection_error(os.str());
        }

        void basic_connection::connect(const connection_options &opts) {
            connectImpl(opts);
        }
        rdalias const &basic_connection::alias() const {
            return aliasImpl();
        }
        void basic_connection::execute(events::execute &&evt) {
            executeImpl(std::move(evt));
        }
        void basic_connection::terminate() {
            terminateImpl();
        }

    } // namespace details
} // namespace redis_async
