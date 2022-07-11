#!/bin/bash

id=$1
id=$(( id + 2 ))
sudo cat /etc/wpa_supplicant/wpa_supplicant-wlx00c0ca365c72.conf | tr '\n' ' ' | cut -d '{' -f$id | cut -d '"' -f4
