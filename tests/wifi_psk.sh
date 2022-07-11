#!/bin/bash

WPA_CLI="sudo wpa_cli -i wlx00c0ca365c72"

ssid=$1
psk=$2

echo "setting new psk ($psk) for ssid $ssid"

id=-1
idx=0
while read curr_ssid; do

    echo $idx. $curr_ssid
    
    if [[ "$curr_ssid" == "$ssid" ]]; then
        id=$idx
        break
    fi

    idx=$(( idx + 1 ))
done <<< $(bash wpa_list.sh)

echo "id: $id"

function wpa_cli {

    echo "executing: $1"
    res=$($WPA_CLI $1)

    if [[ "$res" == "FAIL" ]]; then
        exit -1
    fi

}

if [[ "$id" != -1 ]]; then

    if [[ "$psk" == "" ]]; then
        wpa_cli set_network $id key_mgmt NONE
    else
        wpa_cli set_network $id psk "\"$psk\""
        wpa_cli set_network $id key_mgmt WPA-PSK
    fi

    wpa_cli enable_network $id
    wpa_cli save_config

fi

