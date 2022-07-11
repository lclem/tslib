#!/bin/bash

WPA_CLI="sudo wpa_cli -i wlx00c0ca365c72"

ssid="$1"
psk="$2"

echo "wifi_add called with ssid: $ssid, psk: $psk"

id=$(bash wifi_find.sh "$ssid")
echo "id: $id"

# not found
if [[ $id == -1 ]]; then

    # create new    
    id=$($WPA_CLI add_network)

    $WPA_CLI "set_network $id ssid \"$ssid\""
    $WPA_CLI "set_network $id key_mgmt NONE"
    # $WPA_CLI set_network $id psk "\"$psk\""

# else

#     # set the password again
#     if [[  "$psk" != "" ]]; then
#         $WPA_CLI set_network $id ssid "\"$ssid\""
#         $WPA_CLI set_network $id psk "\"$psk\""
#     fi

fi

$WPA_CLI enable_network $id
$WPA_CLI select_network $id
$WPA_CLI save_config
# wpa_cli reconfigure $id