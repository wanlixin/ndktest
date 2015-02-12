echo --------- deploy %1\libs\armeabi\%2
adb push %1\libs\armeabi\%2 /data/local/tmp
adb shell chmod 555 /data/local/tmp/%2