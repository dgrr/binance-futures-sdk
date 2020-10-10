#!/bin/sh

docker build --rm -t binance-build:v1 .
docker run -v `pwd`:/binance/ binance-build:v1
