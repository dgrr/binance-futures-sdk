FROM ubuntu:20.10

WORKDIR /binance/
RUN apt-get update
RUN apt-get install -y libboost-all-dev libssl-dev cmake build-essential git
CMD ["bash", "-c", "mkdir build; cd build && cmake -DBINANCE_BUILD_EXAMPLES:BOOL=ON .. && cmake --build . --target all -- -j1"]
