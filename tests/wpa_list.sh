#!/bin/bash

while read network
do
    idx=$(echo $network | cut -d ' ' -f1)
    ssid=$(echo $network | cut -d ' ' -f2- | cut -d '[' -f1 | xargs | rev | cut -d ' ' -f2- | rev | xargs)

    echo "$ssid"

done <<< $(sudo wpa_cli -i wlx00c0ca365c72 list_networks | tail -n +2)