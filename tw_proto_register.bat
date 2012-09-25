cd /d %~dp0
echo Windows Registry Editor Version 5.00 > tw.reg

echo [HKEY_CLASSES_ROOT\tw] >> tw.reg
echo "URL Protocol"="" >> tw.reg
echo @="URL:Teeworlds Protocol" >> tw.reg

echo [HKEY_CLASSES_ROOT\tw\DefaultIcon] >> tw.reg
echo @="%~dp0n-client.exe" >> tw.reg

echo [HKEY_CLASSES_ROOT\tw\shell] >> tw.reg
echo @="open" >> tw.reg

echo [HKEY_CLASSES_ROOT\tw\shell\open] >> tw.reg

echo [HKEY_CLASSES_ROOT\tw\shell\open\command] >> tw.reg
echo @="%~dp0tw_proto_start.bat %1" >> tw.reg

tw.reg