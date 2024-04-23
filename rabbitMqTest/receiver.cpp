#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

int main() {
    amqp_connection_state_t conn;
    amqp_socket_t *socket = NULL;
    amqp_rpc_reply_t reply;
    amqp_envelope_t envelope;

    const char* hostname = "localhost";
    const int port = 5672;
    const char* queue_name = "test";

    conn = amqp_new_connection();
    socket = amqp_tcp_socket_new(conn);

    if (!socket) {
        printf("Error creating TCP socket\n");
        return -1;
    }

    int status = amqp_socket_open(socket, hostname, port);
    if (status) {
        printf("Error opening TCP socket\n");
        return -1;
    }

    reply = amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        printf("Error logging in\n");
        return -1;
    }

    amqp_channel_open(conn, 1);
    reply = amqp_get_rpc_reply(conn);
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        printf("Error opening channel\n");
        return -1;
    }

    amqp_basic_consume(conn, 1, amqp_cstring_bytes(queue_name), amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
    reply = amqp_get_rpc_reply(conn);
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        printf("Error consuming from queue\n");
        return -1;
    }
    int i = 0 ;
    while (i<10) {
        amqp_maybe_release_buffers(conn);
        reply = amqp_consume_message(conn, &envelope, NULL, 0);

        if (reply.reply_type == AMQP_RESPONSE_NORMAL) {
            printf("Received message: %.*s\n", (int)envelope.message.body.len, (char*)envelope.message.body.bytes);
            amqp_destroy_envelope(&envelope);
        }
        i++;
    }

    return 0;
}

