#build using an ubuntu image called build
FROM ubuntu:22.04 AS build
#update the apt lists, install compiler and cmake, delete apt lists
RUN apt-get update && apt-get install -y --no-install-recommends g++ cmake make && rm -rf /var/lib/apt/lists/*
#change the working directory to /ga (create the file in the image)
WORKDIR /ga 
#copy the files from my current dir (local) to the image current dir (/ga)
COPY . .
#build the files using cmake 
RUN cmake -S . -B build -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build

#creat new image
FROM ubuntu:22.04

#creat a usr so the code is not run as root 
RUN useradd --no-create-home engine
#copy only the excutable from the previous image 
COPY --from=build /ga/build/tsp_engine /usr/local/bin/tsp_engine
#change user from route to engine
USER engine
#run this command when the container starts
ENTRYPOINT [ "/usr/local/bin/tsp_engine" ]