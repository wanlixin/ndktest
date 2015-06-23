echo --------- deploy %1\libs\armeabi\%2 %1\libs\armeabi\%3

adb push %1\libs\armeabi\%2 /data/local/tmp
adb shell chmod 755 /data/local/tmp/%2

adb push %1\libs\armeabi\%3 /data/local/tmp
adb shell chmod 755 /data/local/tmp/%3