FROM python:3  

# Install additional tools
RUN apt update && apt install -y --no-install-recommends \
    git \
    make \
    gcc \
    g++ \
    && rm -rf /var/lib/apt/lists/*

# Install the required Python packages for PlatformIO.

RUN pip install --upgrade platformio

WORKDIR /project
COPY . /project

CMD ["platformio", "run"]
