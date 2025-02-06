from typing import ReadOnly
from fastapi import FastAPI, HTTPException, Depends, Header
from pydantic import BaseModel
import jwt
import os
import datetime
from fastapi import Depends
from SqlLiteManager import SqlLiteManager, Role
from RabbitManager import RabbitMQService
from contextlib import asynccontextmanager


# Secret key for JWT encoding
SECRET_KEY = "mysecretkey"


# Model for login request
class LoginRequest(BaseModel):
    username: str
    password: str


class CreateUserRequest(BaseModel):
    username: str
    password: str
    role: str


# Model for sending messages
class MessageRequest(BaseModel):
    queue_name: str
    queue_server: str = "localhost:5672"
    username: str
    password: str
    message: str


# dp
def get_db():
    db = SqlLiteManager("Agent.db")
    db.createDb()
    return db


@asynccontextmanager
async def lifespan(app: FastAPI):
    global sqlconnection, rabbitmqManager  # Declare as global
    print("Server is starting up. Checking initial conditions...")
    os.system("source ./Setup.sh")
    initUser = str(os.environ.get("init_admin", "default_username"))
    initUserPass = str(os.environ.get("init_pass", "default_password"))

    sqlconnection = SqlLiteManager("Agent.db")
    sqlconnection.createDb()
    sqlconnection.insert(
        {"username": initUser, "pass": initUserPass, "role": Role.admin.value}
    )

    rmq_host = str(os.environ.get("rmq_host", "default_host"))
    rmq_username = str(os.environ.get("rmq_username", "default_username"))
    rmq_pass = str(os.environ.get("rmq_pass", "default_password"))

    rabbitmqManager = RabbitMQService(rmq_host, rmq_username, rmq_pass)
    rabbitmqManager.connect()

    yield  # This is where FastAPI will run the app

    print("Server is shutting down...")  # Optional cleanup logic here


app = FastAPI(lifespan=lifespan)


# User login and token generation
@app.post("/login")
def login(user: LoginRequest):
    # try to find agent
    auth_user_role = sqlconnection.user_authorazation(user.username, user.password)
    if auth_user_role != None:
        token = jwt.encode(
            {
                "sub": user.username,
                "role": auth_user_role,
                "exp": datetime.datetime.utcnow() + datetime.timedelta(hours=1),
            },
            SECRET_KEY,
            algorithm="HS256",
        )
        return {"token": token}
    raise HTTPException(status_code=401, detail="Invalid credentials")


@app.get("/create_agent")
def create_agent(
    message: CreateUserRequest,
    authorization: str = Header(None),
    db: SqlLiteManager = Depends(get_db),
):
    try:
        token = authorization.split(" ")[1]
        decoded = jwt.decode(token, SECRET_KEY, algorithms=["HS256"])
        username = decoded.get("sub")
        if db.is_admin_user(username):
            role = Role.writeOnly.value if message.role == "w" else Role.readOnly.value
            db.insert(
                {"username": message.username, "pass": message.password, "role": role}
            )
            return {
                "status": "success",
                "message": "user is created use /login to get token",
            }
        else:
            raise HTTPException(
                status_code=403, detail="Only admin users can create agents"
            )
    except jwt.ExpiredSignatureError:
        raise HTTPException(status_code=401, detail="Token expired")
    except jwt.InvalidTokenError:
        raise HTTPException(status_code=401, detail="Invalid token")


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
