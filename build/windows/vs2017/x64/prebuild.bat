set os_version=windows\vs2017\x64\
set dep_base=..\..\..\..\deps\
set bin_base=..\..\..\..\bin\

rmdir /Q /S %bin_base%
rmdir /Q /S %dep_base%

mkdir %dep_base%include
mkdir %dep_base%lib

::Baselib
set src_base=..\..\..\..\..\..\..\..\..\Versions\Baselib\

::los
mkdir %dep_base%include\los
xcopy %src_base%los\v1.0.0\%os_version%include %dep_base%include\los /S /Y /C
xcopy %src_base%los\v1.0.0\%os_version%lib %dep_base%lib /S /Y /C

::libcurl
mkdir %dep_base%include\libcurl
xcopy %src_base%libcurl\v7.78.0\%os_version%include %dep_base%include\libcurl /S /Y /C
xcopy %src_base%libcurl\v7.78.0\%os_version%lib %dep_base%lib /S /Y /C