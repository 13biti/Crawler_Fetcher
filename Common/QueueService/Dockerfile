from registry.docker.ir/library/python:3.11

WORKDIR /app 
env http_proxy=http://192.168.46.238:8080/
env https_proxy=http://192.168.46.238:8080/
copy ./requirements.txt .
run pip install --no-cache-dir -r ./requirements.txt

copy . . 

expose 8001 

ENV init_admin="admin"
ENV init_pass="123"
ENV rmq_host="rabbitmq"
ENV rmq_username="test1"
ENV rmq_pass="123123"

CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8001"]
