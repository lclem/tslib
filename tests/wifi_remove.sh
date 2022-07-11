#!/bin/bash

WPA_CLI="sudo wpa_cli -i wlx00c0ca365c72"

id=$1
$WPA_CLI remove_network $id
$WPA_CLI save_config
# $WPA_CLI reconfigure $id