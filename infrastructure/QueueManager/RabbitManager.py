import pika
import json
import logging
import time

logger = logging.getLogger("rabbitmq_logger")  # Unique logger name
logger.setLevel(logging.INFO)

file_handler = logging.FileHandler("rabbitmq_service.log")
file_handler.setFormatter(
    logging.Formatter("%(asctime)s - %(levelname)s - %(message)s")
)

console_handler = logging.StreamHandler()
console_handler.setFormatter(
    logging.Formatter("%(asctime)s - %(levelname)s - %(message)s")
)

logger.addHandler(file_handler)
logger.addHandler(console_handler)


class RabbitMQService:
    def __init__(self, host="localhost", username="guest", password="guest"):
        self.host = host
        self.username = username
        self.password = password
        self.connection = None
        self.channel = None
        logging.info("RabbitMQService initialized with host: %s", self.host)

    def connect(self):
        """Connect to RabbitMQ server with authentication."""
        try:
            credentials = pika.PlainCredentials(self.username, self.password)
            parameters = pika.ConnectionParameters(
                host=self.host,
                credentials=credentials,
                heartbeat=600,
                blocked_connection_timeout=300,
            )
            self.connection = pika.BlockingConnection(parameters)
            self.channel = self.connection.channel()
            logger.info("Connected to RabbitMQ at %s successfully.", self.host)
            return True
        except Exception as e:
            logger.error("Failed to connect to RabbitMQ: %s", e)
            return False

    def connectionRecovery(self, retries=5, delay=10):
        credentials = pika.PlainCredentials(self.username, self.password)
        parameters = pika.ConnectionParameters(
            host=self.host,
            credentials=credentials,
            heartbeat=600,
            blocked_connection_timeout=300,
        )
        for _ in range(retries):
            try:
                self.connection = pika.BlockingConnection(parameters)
                self.channel = self.connection.channel()
                logger.info("Connected to RabbitMQ at %s successfully.", self.host)
                return True
            except Exception as e:
                logger.error("Failed to connect to RabbitMQ: %s", e)
                time.sleep(delay)
        return False

    def disconnect(self):
        """Disconnect from RabbitMQ server."""
        if self.connection and not self.connection.is_closed:
            self.connection.close()
            logging.info("Disconnected from RabbitMQ successfully.")
        else:
            logging.warning("No active connection to disconnect.")

    def queue_exists(self, queue_name):
        """
        Check if a queue exists.
        :param queue_name: Name of the queue.
        :return: True if queue exists, False otherwise.
        """
        if not self.channel:
            logging.error("No active channel. try to connecctoin recovery ")
            if not self.connectionRecovery():
                return False

        try:
            self.channel.queue_declare(queue=queue_name, passive=True)
            logging.info("Queue '%s' exists.", queue_name)
            return True
        except pika.exceptions.ChannelClosedByBroker:
            logging.warning("Queue '%s' does not exist.", queue_name)
            self.channel = (
                self.connection.channel()
            )  # Reopen the channel after exception
            return False
        except Exception as e:
            logging.error("Error while checking queue existence: %s", e)
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
            logging.error("No active channel. try to connecctoin recovery ")
            if not self.connectionRecovery():
                return False

        if self.queue_exists(queue_name):
            logging.info("Queue '%s' already exists. Skipping creation.", queue_name)
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
            logging.info(
                "Queue '%s' created successfully with arguments: %s",
                queue_name,
                arguments,
            )
        except Exception as e:
            logging.error("Failed to create queue: %s", e)

    def add_item_to_queue(self, queue_name, item):
        """Add an item to the specified queue."""
        if not self.channel:
            logging.error("No active channel. try to connecctoin recovery ")
            if not self.connectionRecovery():
                return False

        try:
            self.channel.basic_publish(
                exchange="",
                routing_key=queue_name,
                body=json.dumps(item),
                properties=pika.BasicProperties(
                    delivery_mode=2,  # Make message persistent
                ),
            )
            logging.info("Item added to queue '%s': %s", queue_name, item)
        except Exception as e:
            logging.error("Failed to add item to queue: %s", e)

    def remove_item_from_queue(self, queue_name, auto_ack=True):
        """
        Remove a single item from the specified queue.
        :param queue_name: Name of the queue.
        :param auto_ack: If True, the message will be automatically acknowledged.
        :return: The item removed from the queue, or None if the queue is empty.
        """
        if not self.channel:
            logging.error("No active channel. try to connecctoin recovery ")
            if not self.connectionRecovery():
                return False

        try:
            method_frame, header_frame, body = self.channel.basic_get(
                queue=queue_name, auto_ack=auto_ack
            )
            if method_frame:
                item = json.loads(body)
                logging.info("Item removed from queue '%s': %s", queue_name, item)
                return item
            else:
                logging.info("No items in queue '%s'.", queue_name)
                return None
        except Exception as e:
            logging.error("Failed to remove item from queue: %s", e)
            return None
