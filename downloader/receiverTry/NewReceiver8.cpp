#include <iostream>
#include <string>
#include <amqp.h>
#include <amqp_tcp_socket.h>

std::string receiveAndRemoveMessage(const std::string& queue_name) {
    amqp_connection_state_t conn = amqp_new_connection();
    amqp_socket_t* socket = amqp_tcp_socket_new(conn);

    if (!socket) {
        std::cerr << "Error creating TCP socket" << std::endl;
        return "";
    }

    if (amqp_socket_open(socket, "localhost", 5672)) {
        std::cerr << "Error opening TCP socket" << std::endl;
        return "";
    }

    amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");
    amqp_channel_open(conn, 1);

    amqp_rpc_reply_t res;
    res = amqp_get_rpc_reply(conn);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        std::cerr << "Error opening channel: " << res.reply_type << std::endl;
        return "";
    }

    amqp_basic_qos(conn, 1, 0, 1, 0);

    amqp_bytes_t queuename = amqp_cstring_bytes(queue_name.c_str());
    amqp_basic_consume(conn, 1, queuename, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);

    res = amqp_get_rpc_reply(conn);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        std::cerr << "Error consuming from queue: " << res.reply_type << std::endl;
        return "";
    }

    amqp_envelope_t envelope;
    amqp_maybe_release_buffers(conn);
    res = amqp_consume_message(conn, &envelope, NULL, 0);

    if (res.reply_type == AMQP_RESPONSE_NORMAL) {
        std::string received_message(reinterpret_cast<char*>(envelope.message.body.bytes), envelope.message.body.len);
        amqp_destroy_envelope(&envelope);

        amqp_basic_ack(conn, 1, envelope.delivery_tag, 0);

        amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
        amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(conn);

        std::cout << "Received message: " << received_message << std::endl;
        return received_message;
    } else {
        std::cerr << "Error receiving and removing message" << std::endl;
        return "";
    }
}

int main() {
    std::string queue_name = "test";

    std::string received_message = receiveAndRemoveMessage(queue_name);

    if (!received_message.empty()) {
        std::cout << "Received and removed message: " << received_message << std::endl;
    } else {
        std::cerr << "Failed to receive and remove message" << std::endl;
    }

    return 0;
}

