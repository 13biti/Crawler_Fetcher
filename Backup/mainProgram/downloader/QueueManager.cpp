#include "QueueManager.h"

QueueManager::QueueManager(const char* hostname, int port) {
    conn = amqp_new_connection();
    socket = amqp_tcp_socket_new(conn);
    
    if (!socket) {
        std::cerr << "Error creating TCP socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    int status = amqp_socket_open(socket, hostname, port);
    if (status) {
        std::cerr << "Error opening TCP socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    amqp_rpc_reply_t reply = amqp_login(conn, "/",0, 131072,
                                        0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        std::cerr << "Error logging in" << std::endl;
        exit(EXIT_FAILURE);
    }

    amqp_channel_open(conn, 1);
    reply = amqp_get_rpc_reply(conn);
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        std::cerr << "Error opening channel" << std::endl;
        exit(EXIT_FAILURE);
    }
}

QueueManager::~QueueManager() {
    amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(conn);
}

bool QueueManager::createQueue(const std::string& queueName) {
    amqp_queue_declare_ok_t* r = amqp_queue_declare(
        conn, 1,
        amqp_cstring_bytes(queueName.c_str()),
        0, 0, 0, 0, amqp_empty_table);

    if (r == NULL) {
        std::cerr << "Error declaring queue" << std::endl;
        return false;
    }
    return true;
}

bool QueueManager::sendMessage(const std::string& queue_name, const std::string& message) {
    amqp_bytes_t body = amqp_cstring_bytes(message.c_str());

    amqp_basic_properties_t props;
    props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
    props.content_type = amqp_cstring_bytes("text/plain");
    props.delivery_mode = 2; 
    int status = amqp_basic_publish(conn, 1, amqp_cstring_bytes(""), 
        amqp_cstring_bytes(queue_name.c_str()), 0, 0, &props, body);
    return status == AMQP_STATUS_OK;
}
std::string QueueManager::receiveMessage(const std::string& queue_name) {
    amqp_rpc_reply_t res;
    amqp_message_t message;
    amqp_envelope_t envelope;

    res = amqp_basic_get(conn, 1, amqp_cstring_bytes(queue_name.c_str()), 1);
    if (res.reply_type == AMQP_RESPONSE_NORMAL) {
        if (amqp_read_message(conn, 1, &message, 0)) {
            std::string received_message(reinterpret_cast<char*>(message.body.bytes), message.body.len);
            amqp_destroy_message(&message);

            // Acknowledge the message
            amqp_basic_ack(conn, 1, envelope.delivery_tag, 0);

            return received_message;
        }
    }

    return ""; // Return empty string if no message received
}

/*
std::string QueueManager::receiveMessage(const std::string& queue_name) {
    amqp_envelope_t envelope;
    amqp_basic_consume(conn, 1, amqp_cstring_bytes(queue_name.c_str()),
                       amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
    amqp_rpc_reply_t reply = amqp_get_rpc_reply(conn);
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        std::cerr << "Error consuming from queue" << std::endl;
        exit(EXIT_FAILURE);
    }

    amqp_maybe_release_buffers(conn);
    reply = amqp_consume_message(conn, &envelope, NULL, 0);
    if (reply.reply_type == AMQP_RESPONSE_NORMAL) {
        std::string message(reinterpret_cast<char*>(envelope.message.body.bytes), envelope.message.body.len);
        amqp_destroy_envelope(&envelope);

        // Acknowledge the message
        amqp_basic_ack(conn, 1, envelope.delivery_tag, 0);

        return message;
    } else {
        return ""; // Return empty string if no message received
    }
}
std::string QueueManager::receiveMessage(const std::string& queue_name) {
    amqp_envelope_t envelope;
    amqp_basic_consume(conn, 1, amqp_cstring_bytes(queue_name.c_str()),
                       amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
    amqp_rpc_reply_t reply = amqp_get_rpc_reply(conn);
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        std::cerr << "Error consuming from queue" << std::endl;
        exit(EXIT_FAILURE);
    }

    amqp_maybe_release_buffers(conn);
    reply = amqp_consume_message(conn, &envelope, NULL, 0);
    if (reply.reply_type == AMQP_RESPONSE_NORMAL) {
        std::string message(reinterpret_cast<char*>(envelope.message.body.bytes), envelope.message.body.len);
        amqp_destroy_envelope(&envelope);
        return message;
    } else {
        return ""; // Return empty string if no message received
    }
}
*/
amqp_connection_state_t QueueManager::getConnection() {
    return conn;
}

