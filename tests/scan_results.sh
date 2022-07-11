#!/bin/bash

sudo wpa_cli -i wlx00c0ca365c72 scan_results | tail -n +2 | while read line
do
    line=$(echo $line | cut -f1)
    bssid=$(echo $line | cut -d ' ' -f1)
    dbm=$(echo $line | cut -d ' ' -f3)
    opts=$(echo $line | cut -d ' ' -f4)
    name=$(echo $line | cut -d ' ' -f5-)
    #echo "$line"
    echo $dbm $name
done | sort -k1


