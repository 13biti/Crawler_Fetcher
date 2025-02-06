from fastapi import FastAPI, HTTPException, Depends, Header
from pydantic import BaseModel
import jwt
import os 
import datetime
from SqlLiteManager import SqlLiteManager , Role
from RabbitManager import RabbitMQService

app = FastAPI()

# Secret key for JWT encoding
SECRET_KEY = "mysecretkey"



# Model for login request
class LoginRequest(BaseModel):
    username: str
    password: str


# Model for sending messages
class MessageRequest(BaseModel):
    queue_name: str
    queue_server: str = "localhost:5672"
    username: str
    password: str
    message: str


sqlconnection = None
rabbitmqManager = None
@app.on_event("startup")
async def startup_event():
    print("Server is starting up. Checking initial conditions...")
    initUser = str(os.environ.get("init_admin", "default_username"))
    initUserPass = str(os.environ.get("init_pass", "default_password"))

    sqlconnection = SqlLiteManager('Agent.db')
    sqlconnection.createDb()

    sqlconnection.insert({
        "username": initUser,
        "pass": initUserPass,
        "role": Role.admin
    })

    rmq_host = str(os.environ.get("rmq_host", "default_host"))
    rmq_username = str(os.environ.get("rmq_username", "default_username"))
    rmq_pass = str(os.environ.get("rmq_pass", "default_password"))

    # Create an instance of RabbitMQService with the fetched environment variables
    rabbitmqManager = RabbitMQService(rmq_host, rmq_username, rmq_pass)
    rabbitmqManager.connect()


# User login and token generation
@app.post("/login")
def login(user: LoginRequest):
    if USERS_DB.get(user.username) == user.password:
        token = jwt.encode(
            {
                "sub": user.username,
                "role": user.role
                "exp": datetime.datetime.utcnow() + datetime.timedelta(hours=1),
            },
            SECRET_KEY,
            algorithm="HS256",
        )
        return {"token": token}
    raise HTTPException(status_code=401, detail="Invalid credentials")

# Simulated message sending
@app.post("/send-message")
def send_message(message: MessageRequest, authorization: str = Header(None)):
    try:
        token = authorization.split(" ")[1]  # Extract token from "Bearer <token>"
        decoded = jwt.decode(token, SECRET_KEY, algorithms=["HS256"])
        print(decoded.get("exp"))
        return {
            "status": "success",
            "queue_name": message.queue_name,
            "queue_server": message.queue_server,
            "message": message.message,
        }
    except jwt.ExpiredSignatureError:
        raise HTTPException(status_code=401, detail="Token expired")
    except jwt.InvalidTokenError:
        raise HTTPException(status_code=401, detail="Invalid token")
