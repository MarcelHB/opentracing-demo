FROM python:2.7

RUN mkdir /service
WORKDIR /service

COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

