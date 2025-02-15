import sqlite3
import logging
from enum import Enum
import threading

# Configure logging
logging.basicConfig(
    filename="database.log",  # Log file name
    level=logging.INFO,  # Log level
    format="%(asctime)s - %(levelname)s - %(message)s",  # Log format
)


class Role(Enum):
    admin = 0
    readOnly = 1
    writeOnly = 2


class SqlLiteManager:
    def __init__(self, filename: str) -> None:
        self.servername = filename
        self.local = threading.local()
        logging.info(f"Initialized SqlLiteManager with database file: {filename}")

    def get_connection(self):
        if not hasattr(self.local, "connection"):
            self.local.connection = sqlite3.connect(self.servername)
        return self.local.connection

    def createDb(self):
        """Creates the database and the Agents table if it doesn't exist."""
        try:
            connection = self.get_connection()
            cursor = connection.cursor()
            cursor.execute("""
            CREATE TABLE IF NOT EXISTS Agents (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT NOT NULL,
                pass TEXT NOT NULL,
                role INTEGER NOT NULL
            )
            """)
            connection.commit()
            logging.info("Database and table 'Agents' created (if not exist).")
        except sqlite3.Error as e:
            logging.error(f"Error creating database: {e}")

    def is_admin_user(self, username: str) -> bool:
        connection = self.get_connection()
        try:
            cursor = connection.cursor()
            logging.info(f"Try to check role by u:{username}")
            cursor.execute(
                "SELECT * FROM Agents WHERE username = ? AND role = ?",
                (
                    username,
                    Role.admin.value,
                ),
            )
            row = cursor.fetchone()
            logging.info(f"checking result : {row}")
            return row is not None
        except sqlite3.Error as e:
            logging.error(f"Error checking admin user: {e}")
            return False

    def insert(self, data: dict):
        """Inserts a new record into the Agents table."""
        connection = self.get_connection()
        try:
            cursor = connection.cursor()
            cursor.execute(
                """
                INSERT INTO Agents (username, pass, role)
                VALUES (:username, :pass, :role)
                """,
                data,
            )
            connection.commit()
            logging.info(f"Inserted record: {data}")
        except sqlite3.Error as e:
            logging.error(f"Error inserting record {data}: {e}")

    def read_by_id(self, id: int):
        """Fetches a record by ID."""
        connection = self.get_connection()
        try:
            cursor = connection.cursor()
            cursor.execute("SELECT * FROM Agents WHERE id = ?", (id,))
            row = cursor.fetchone()

            if row:
                logging.info(f"Record found with ID {id}: {row}")
                return {
                    "id": row[0],
                    "username": row[1],
                    "pass": row[2],
                    "role": row[3],
                }
            else:
                logging.warning(f"No record found with ID {id}")
                return None
        except sqlite3.Error as e:
            logging.error(f"Error fetching record by ID {id}: {e}")
            return None

    def find_by_username(self, username: str):
        """Fetches a record by username."""
        connection = self.get_connection()
        try:
            cursor = connection.cursor()
            cursor.execute("SELECT * FROM Agents WHERE username = ?", (username,))
            row = cursor.fetchone()

            if row:
                logging.info(f"Record found with username '{username}': {row}")
                return {
                    "id": row[0],
                    "username": row[1],
                    "pass": row[2],
                    "role": row[3],
                }
            else:
                logging.warning(f"No record found with username '{username}'")
                return None
        except sqlite3.Error as e:
            logging.error(f"Error fetching record by username '{username}': {e}")
            return None

    def user_authorazation(self, username, password):
        connection = self.get_connection()
        try:
            cursor = connection.cursor()
            logging.info(f"Try to authenticate by u:{username} , p:{password}")
            cursor.execute(
                "SELECT * FROM Agents WHERE username = ? AND pass = ?",
                (
                    username,
                    password,
                ),
            )
            row = cursor.fetchone()
            if row:
                logging.info(f"auth result : {row}")
                if row[3] == Role.admin.value:
                    return Role.admin.value
                elif row[3] == Role.readOnly.value:
                    return Role.readOnly.value
                else:
                    return Role.writeOnly.value
            else:
                logging.warning(f"No record found with username '{username}'")
                return None
        except sqlite3.Error as e:
            logging.error(f"Error fetching record by username '{username}': {e}")
            return None

    def close(self):
        """Closes the database connection."""
        if hasattr(self.local, "connection"):
            self.local.connection.close()
            logging.info("Database connection closed.")
        else:
            logging.warning("Attempted to close a non-existent connection.")
