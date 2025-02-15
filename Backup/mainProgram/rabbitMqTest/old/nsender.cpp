#include <iostream>
#include <amqp.h>
#include <amqp_tcp_socket.h>

int main() {
    amqp_connection_state_t conn;
    conn = amqp_new_connection();

    amqp_socket_t *socket = amqp_tcp_socket_new(conn);
    if (!socket) {
        std::cerr << "Error creating TCP socket" << std::endl;
        return 1;
    }

    int status = amqp_socket_open(socket, "localhost", 5672);
    if (status) {
        std::cerr << "Error opening socket" << std::endl;
        return 1;
    }

    amqp_rpc_reply_t login_reply = amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");
    if (login_reply.reply_type != AMQP_RESPONSE_NORMAL) {
        std::cerr << "Error logging in" << std::endl;
        return 1;
    }

    amqp_channel_open(conn, 1);
    amqp_get_rpc_reply(conn);

    // Declare a queue named "test"
    amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, 1, amqp_cstring_bytes("test"), 0, 0, 0, 0, amqp_empty_table);
    if (r == NULL) {
        std::cerr << "Error declaring queue" << std::endl;
        return 1;
    }

    amqp_basic_properties_t props;
    props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
    props.content_type = amqp_cstring_bytes("text/plain");
    props.delivery_mode = 2; // persistent delivery mode

    std::string message = "Hello, World!";
    amqp_bytes_t body = amqp_cstring_bytes(message.c_str());

    status = amqp_basic_publish(conn, 1, amqp_cstring_bytes(""), amqp_cstring_bytes("test"), 0, 0, &props, body);
    if (status) {
        std::cerr << "Error publishing message" << std::endl;
        return 1;
    }

    amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
    amqp_connection_close(conn, AMQP_REPLY_SUCCESS);

    amqp_destroy_connection(conn);

    return 0;
}

