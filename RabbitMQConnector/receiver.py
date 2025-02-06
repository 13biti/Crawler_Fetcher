import pika
import sys


def receive_message(queue_name):
    # Connect to RabbitMQ
    connection = pika.BlockingConnection(pika.ConnectionParameters("localhost"))
    channel = connection.channel()

    # Declare a durable queue
    channel.queue_declare(queue=queue_name, durable=True)

    # Set up a consumer
    def callback(ch, method, properties, body):
        print(f" [x] Received '{body.decode()}' from queue '{queue_name}'")
        ch.basic_ack(delivery_tag=method.delivery_tag)  # Acknowledge the message

    channel.basic_consume(queue=queue_name, on_message_callback=callback)

    print(f" [*] Waiting for messages in queue '{queue_name}'. To exit, press CTRL+C")
    channel.start_consuming()


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: receiver.py <queue_name>")
        sys.exit(1)

    queue_name = sys.argv[1]
    receive_message(queue_name)
