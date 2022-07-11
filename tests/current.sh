#!/bin/bash

# network id / ssid / bssid / flags
# 0	UPCC51A4A6-2.4	any	[CURRENT]
# 1	LEEEEEEROY	any	[DISABLED]
# 2	R36A-AE3B73	any	[DISABLED]
# 3	muratico	any	[DISABLED]

current=$(sudo wpa_cli -i wlx00c0ca365c72 list_networks | grep "CURRENT")
idx=$(echo $current | cut -d ' ' -f1)
name=$(echo $current | cut -d ' ' -f2- | rev | cut -d ' ' -f3- | rev | xargs)

echo $current
echo "$idx $name"
