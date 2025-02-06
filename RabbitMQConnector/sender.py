import pika
import sys


def send_message(queue_name, message):
    # Connect to RabbitMQ
    connection = pika.BlockingConnection(pika.ConnectionParameters("localhost"))
    channel = connection.channel()

    # Declare a durable queue
    channel.queue_declare(queue=queue_name, durable=True)

    # Publish the message
    channel.basic_publish(
        exchange="",
        routing_key=queue_name,
        body=message,
        properties=pika.BasicProperties(
            delivery_mode=2,  # Make the message persistent
        ),
    )
    print(f" [x] Sent '{message}' to queue '{queue_name}'")

    # Close the connection
    connection.close()


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: sender.py <queue_name> <message>")
        sys.exit(1)

    queue_name = sys.argv[1]
    message = sys.argv[2]
    send_message(queue_name, message)
