#!/bin/bash

# network id / ssid / bssid / flags
# 0	UPCC51A4A6-2.4	any	[CURRENT]
# 1	LEEEEEEROY	any	[DISABLED]
# 2	R36A-AE3B73	any	[DISABLED]
# 3	muratico	any	[DISABLED]

wifi=$1

idx=0
while read curr_ssid; do

    echo $idx. $curr_ssid
    
    if [[ "$curr_ssid" == "$ssid" ]]; then
        echo 1
        exit 1
    fi

    idx=$(( idx + 1 ))
done <<< $(bash wpa_list.sh)

echo 0
exit 0