#!/bin/bash

IFCONFIG="ifconfig wlx00c0ca365c72"

network=$(iwgetid | cut -d':' -f2 | cut -d'"' -f2 | tr -d '\n')
ip=$($IFCONFIG | grep inet | xargs | cut -d " " -f2)

if [[ "$network" != "" ]]; then
    echo -n "$network ($ip)"
fi