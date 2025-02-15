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
    amqp_basic_get(conn, 1, amqp_cstring_bytes(queue_name.c_str()), 1);

    amqp_rpc_reply_t res;
    amqp_message_t message;
    int status = amqp_read_message(conn, 1, &message, 0);

    if (status == AMQP_STATUS_OK) {
        std::string received_message(reinterpret_cast<char*>(message.body.bytes), message.body.len);
        amqp_destroy_message(&message);

        amqp_basic_ack(conn, 1, message.delivery_tag, 0);

        amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
        amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(conn);

        return received_message;
    } else {
        std::cerr << "Error reading message" << std::endl;
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

