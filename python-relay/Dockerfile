FROM python:3

COPY requirements.txt ./
COPY relay.py ./

RUN pip install --no-cache-dir -r requirements.txt

ENTRYPOINT [ "python", "./relay.py" ]
