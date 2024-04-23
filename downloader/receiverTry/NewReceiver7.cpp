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

    amqp_basic_get(conn, 1, amqp_cstring_bytes(queue_name.c_str()), 1);

    res = amqp_get_rpc_reply(conn);
    if (res.reply_type != AMQP_RESPONSE_NORMAL) {
        std::cerr << "Error getting message: " << res.reply_type << std::endl;
        return "";
    }

    amqp_envelope_t envelope;
    res = amqp_consume_message(conn, &envelope, NULL, 0);

    if (res.reply_type == AMQP_RESPONSE_NORMAL) {
        std::string received_message(reinterpret_cast<char*>(envelope.message.body.bytes), envelope.message.body.len);
        amqp_destroy_envelope(&envelope);

        // Acknowledge the message to remove it from the queue
        amqp_basic_ack(conn, 1, envelope.delivery_tag, 0);

        amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
        amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(conn);

        return received_message;
    } else {
        std::cerr << "Error receiving and removing message" << std::endl;
        return ""; // Return empty string if no message received
    }
}

