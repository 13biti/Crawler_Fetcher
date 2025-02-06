import pika
import json


class RabbitMQService:
    def __init__(self, host="localhost", username="guest", password="guest"):
        self.host = host
        self.username = username
        self.password = password
        self.connection = None
        self.channel = None

    def connect(self):
        """Connect to RabbitMQ server with authentication."""
        try:
            credentials = pika.PlainCredentials(self.username, self.password)
            parameters = pika.ConnectionParameters(
                host=self.host, credentials=credentials
            )
            self.connection = pika.BlockingConnection(parameters)
            self.channel = self.connection.channel()
            print(f"Connected to RabbitMQ at {self.host} successfully.")
        except Exception as e:
            print(f"Failed to connect to RabbitMQ: {e}")

    def disconnect(self):
        """Disconnect from RabbitMQ server."""
        if self.connection and not self.connection.is_closed:
            self.connection.close()
            print("Disconnected from RabbitMQ successfully.")
        else:
            print("No active connection to disconnect.")

    def queue_exists(self, queue_name):
        """
        Check if a queue exists.
        :param queue_name: Name of the queue.
        :return: True if queue exists, False otherwise.
        """
        if not self.channel:
            print("Error: No active channel. Call connect() first.")
            return False

        try:
            self.channel.queue_declare(queue=queue_name, passive=True)
            print(f"Queue '{queue_name}' exists.")
            return True
        except pika.exceptions.ChannelClosedByBroker:
            print(f"Queue '{queue_name}' does not exist.")
            self.channel = (
                self.connection.channel()
            )  # Reopen the channel after exception
            return False
        except Exception as e:
            print(f"Error while checking queue existence: {e}")
            return False

    def create_queue(self, queue_name, durable=True, ttl=None, max_length=None):
        """
        Create a new queue if it doesn't already exist.
        :param queue_name: Name of the queue.
        :param durable: If True, the queue will survive server restarts.
        :param ttl: Time-To-Live for messages in the queue (in milliseconds).
        :param max_length: Maximum number of messages in the queue.
        """
        if not self.channel:
            print("Error: No active channel. Call connect() first.")
            return

        if self.queue_exists(queue_name):
            print(f"Queue '{queue_name}' already exists. Skipping creation.")
            return

        try:
            arguments = {}
            if ttl:
                arguments["x-message-ttl"] = ttl  # Set message TTL
            if max_length:
                arguments["x-max-length"] = max_length  # Set max queue length

            self.channel.queue_declare(
                queue=queue_name, durable=durable, arguments=arguments
            )
            print(
                f"Queue '{queue_name}' created successfully with arguments: {arguments}"
            )
        except Exception as e:
            print(f"Failed to create queue: {e}")

    def add_item_to_queue(self, queue_name, item):
        """Add an item to the specified queue."""
        if not self.channel:
            print("Error: No active channel. Call connect() first.")
            return

        try:
            self.channel.basic_publish(
                exchange="",
                routing_key=queue_name,
                body=json.dumps(item),
                properties=pika.BasicProperties(
                    delivery_mode=2,  # Make message persistent
                ),
            )
            print(f"Item added to queue '{queue_name}': {item}")
        except Exception as e:
            print(f"Failed to add item to queue: {e}")

    def remove_item_from_queue(self, queue_name, auto_ack=True):
        """
        Remove a single item from the specified queue.
        :param queue_name: Name of the queue.
        :param auto_ack: If True, the message will be automatically acknowledged.
        :return: The item removed from the queue, or None if the queue is empty.
        """
        if not self.channel:
            print("Error: No active channel. Call connect() first.")
            return None

        try:
            method_frame, header_frame, body = self.channel.basic_get(
                queue=queue_name, auto_ack=auto_ack
            )
            if method_frame:
                item = json.loads(body)
                print(f"Item removed from queue '{queue_name}': {item}")
                return item
            else:
                print(f"No items in queue '{queue_name}'.")
                return None
        except Exception as e:
            print(f"Failed to remove item from queue: {e}")
            return None


# Example usage:
if __name__ == "__main__":
    rabbitmq_service = RabbitMQService(
        host="localhost", username="guest", password="guest"
    )
    rabbitmq_service.connect()

    # Create a queue only if it doesn't already exist
    rabbitmq_service.create_queue(
        "test_queue", ttl=60000, max_length=10
    )  # TTL = 60 seconds, max length = 10

    # Add items to the queue
    rabbitmq_service.add_item_to_queue("test_queue", {"key1": "value1"})
    rabbitmq_service.add_item_to_queue("test_queue", {"key2": "value2"})

    # Remove a single item from the queue
    item = rabbitmq_service.remove_item_from_queue("test_queue")
    print(f"Removed item: {item}")

    rabbitmq_service.disconnect()
