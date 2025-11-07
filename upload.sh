#!/bin/bash

echo starting
while [ -n "$?" ]; do
  esphome upload ./thebox3.yaml --device /dev/ttyUSB1
done
