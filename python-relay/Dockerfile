FROM python:2

COPY requirements.txt ./
COPY relay.py ./

RUN pip install --no-cache-dir -r requirements.txt

CMD [ "python", "./relay.py" ]
