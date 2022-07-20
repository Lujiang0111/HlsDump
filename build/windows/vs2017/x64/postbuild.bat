set os_version=windows\vs2017\x64\
set dst_base=..\..\..\..\bin\

::Baselib
set src_base=..\..\..\..\..\..\..\..\..\Versions\Baselib\

::los
xcopy %src_base%los\v1.0.0\%os_version%lib %dst_base% /S /Y /C

::libcurl
xcopy %src_base%libcurl\v7.78.0\%os_version%lib %dst_base% /S /Y /C