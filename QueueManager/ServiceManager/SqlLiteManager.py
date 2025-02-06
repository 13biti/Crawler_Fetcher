import sqlite3
import logging
from enum import Enum

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
        self.connection = None
        logging.info(f"Initialized SqlLiteManager with database file: {filename}")

    def createDb(self):
        """Creates the database and the Agents table if it doesn't exist."""
        try:
            self.connection = sqlite3.connect(self.servername)
            cursor = self.connection.cursor()
            cursor.execute("""
            CREATE TABLE IF NOT EXISTS Agents (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT NOT NULL,
                pass TEXT NOT NULL,
                role INTEGER NOT NULL
            )
            """)
            self.connection.commit()
            logging.info("Database and table 'Agents' created (if not exist).")
        except sqlite3.Error as e:
            logging.error(f"Error creating database: {e}")

    def insert(self, data: dict):
        """Inserts a new record into the Agents table."""
        if not self.connection:
            logging.warning("Database connection not established. Creating database.")
            self.createDb()

        try:
            cursor = self.connection.cursor()
            cursor.execute(
                """
                INSERT INTO Agents (username, pass, role)
                VALUES (:username, :pass, :role)
                """,
                data,
            )
            self.connection.commit()
            logging.info(f"Inserted record: {data}")
        except sqlite3.Error as e:
            logging.error(f"Error inserting record {data}: {e}")

    def read_by_id(self, id: int) -> dict:
        """Fetches a record by ID."""
        if not self.connection:
            logging.warning("Database connection not established. Creating database.")
            self.createDb()

        try:
            cursor = self.connection.cursor()
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

    def find_by_username(self, username: str) -> dict:
        """Fetches a record by username."""
        if not self.connection:
            logging.warning("Database connection not established. Creating database.")
            self.createDb()

        try:
            cursor = self.connection.cursor()
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

    def close(self):
        """Closes the database connection."""
        if self.connection:
            self.connection.close()
            logging.info("Database connection closed.")
        else:
            logging.warning("Attempted to close a non-existent connection.")
