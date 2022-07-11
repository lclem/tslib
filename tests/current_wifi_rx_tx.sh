#!/bin/bash

IFCONFIG="ifconfig wlx00c0ca365c72"

network=$(iwgetid | cut -d':' -f2 | cut -d'"' -f2 | tr -d '\n')
ip=$($IFCONFIG | grep inet | xargs | cut -d " " -f2)
RX=$($IFCONFIG | grep "RX packets" | cut -d'(' -f2 | tr -d ')\n')
TX=$($IFCONFIG | grep "TX packets" | cut -d'(' -f2 | tr -d ')\n')

if [[ "$network" != "" ]]; then
    echo -n "$network [RX $RX / TX $TX] ($ip)"
fi