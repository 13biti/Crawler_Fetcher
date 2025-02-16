from python:3.11

WORKDIR /app 

copy requirments.txt .
run pip install -r --no-cache-dir requirments.txt

copy . . 

expose 8001 

ENV init_admin="admin"
ENV init_pass="123"
ENV rmq_host="localhost"
ENV rmq_username="test1"
ENV rmq_pass="123123"

CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]
