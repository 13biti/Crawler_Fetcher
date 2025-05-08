from fastapi import FastAPI, HTTPException, Depends, Header
from pydantic import BaseModel
import jwt
import os
import datetime
from fastapi import Depends
from SqlLiteManager import SqlLiteManager, Role
from RabbitManager import RabbitMQService
from contextlib import asynccontextmanager
import time


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
class SendMessageRequest(BaseModel):
    queue_name: str
    message: str


class GetMessageRequest(BaseModel):
    queue_name: str


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

    if not rabbitmqManager.connect():
        rabbitmqManager.connectionRecovery()

    yield


app = FastAPI(lifespan=lifespan)


@app.post("/login")
def login(user: LoginRequest):
    auth_user_role = sqlconnection.user_authorazation(user.username, user.password)
    if auth_user_role != None:
        token = jwt.encode(
            {
                "sub": user.username,
                "role": auth_user_role,
                "exp": datetime.datetime.utcnow() + datetime.timedelta(hours=24),
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


# This method should change in future , should have queue auth in token , like get queue name
# Simulated message sending
@app.post("/write")
def write(message: SendMessageRequest, authorization: str = Header(None)):
    try:
        token = authorization.split(" ")[1]  # Extract token from "Bearer <token>"
        decoded = jwt.decode(token, SECRET_KEY, algorithms=["HS256"])
        if decoded.get("role") == Role.writeOnly.value:
            rabbitmqManager.create_queue(message.queue_name, durable=True)
            rabbitmqManager.add_item_to_queue(message.queue_name, message.message)
            return {
                "status": "success",
            }
        else:
            raise HTTPException(status_code=401, detail="You do not have access.")
    except jwt.ExpiredSignatureError:
        raise HTTPException(status_code=401, detail="Token expired")
    except jwt.InvalidTokenError:
        raise HTTPException(status_code=401, detail="Invalid token")


@app.post("/read")
def read(message: GetMessageRequest, authorization: str = Header(None)):
    try:
        token = authorization.split(" ")[1]  # Extract token from "Bearer <token>"
        decoded = jwt.decode(token, SECRET_KEY, algorithms=["HS256"])
        if decoded.get("role") == Role.readOnly.value:
            message = rabbitmqManager.remove_item_from_queue(message.queue_name)
            print(message)
            return {
                "status": "success",
                "message": message,
            }
        else:
            raise HTTPException(status_code=401, detail="You do not have access.")
    except jwt.ExpiredSignatureError:
        raise HTTPException(status_code=401, detail="Token expired")
    except jwt.InvalidTokenError:
        raise HTTPException(status_code=401, detail="Invalid token")
