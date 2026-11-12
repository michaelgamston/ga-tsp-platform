#the python image is built on debian and has the c++ runtime environment already
FROM python:3.11-slim AS build
#update the apt lists, install compiler and cmake, delete apt lists
RUN apt-get update && apt-get install -y --no-install-recommends g++ cmake make && rm -rf /var/lib/apt/lists/*
#change the working directory to /ga (create the file in the image)
WORKDIR /ga 
#copy the files from my current dir (local) to the image current dir (/ga)
COPY tsp_engine .
#build the files using cmake 
RUN cmake -S . -B build -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build

#creat new image
FROM python:3.11-slim

#creat a usr so the code is not run as root 
RUN useradd --no-create-home engine

WORKDIR /app

COPY worker/requirements.txt .

RUN pip install --no-cache-dir -r requirements.txt

#copy only the excutable from the previous image 
COPY --from=build /ga/build/tsp_engine /usr/local/bin/tsp_engine
COPY worker/worker.py /app

ENV ENGINE_PATH=/usr/local/bin/tsp_engine
ENV PYTHONUNBUFFERED=1

#change user from route to engine
USER engine
#run this command when the container starts
ENTRYPOINT [ "python3", "/app/worker.py"]