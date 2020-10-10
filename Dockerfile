FROM ubuntu:20.10

WORKDIR /binance/
RUN apt-get update
RUN apt-get install -y libboost-all-dev libssl-dev cmake build-essential git ninja-build
CMD ["bash", "-c", "mkdir build; cd build && cmake -DBINANCE_DISABLE_THREADING:BOOL=ON -DBINANCE_USE_STRING_VIEW:BOOL=ON -DBINANCE_BUILD_EXAMPLES:BOOL=ON -GNinja .. && cmake --build . --target all -- -j1"]
