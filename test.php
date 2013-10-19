<?php

$res = nfc_open();
if ($res) {
    print "Connected to device with name: " . nfc_device_get_name($res) . "\n";
}

while (1) {
    $nt = nfc_poll($res, 2);
    if ($nt) {
        var_dump($nt);
        $uid = nfc_target_get_uid($nt);
        if ($uid) {
            print "Found card with UID: $uid\n";
        }
    } else {
        print "No card found this poll\n";
    }
    break;
}
