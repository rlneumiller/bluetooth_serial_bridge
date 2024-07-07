FROM espressif/idf:latest

RUN mkdir -p /workspace
RUN apt update && apt install -y python3-pip git nano unzip bash && apt clean

WORKDIR /workspace

#COPY esp32-s3-devkitc1-n16r8.json /root/.platformio/platforms/espressif32/boards

CMD ["bash"] # For debugging container config