@echo off
rem	Check that the user	properly called	this script.

rem	设置APPDIR环境变量为当前服务器程序运行目录
call :compute_pwd
set	APPDIR=%~dp0%
echo 设置工作目录成功

echo "开始卸载 screen-capture-recorder插件"

regsvr32 /u %APPDIR%screen-capture-recorder.dll
regsvr32 /u %APPDIR%audio_sniffer.dll

echo "卸载完成"

:compute_pwd
@FOR /F	"tokens=*" %%i in ('cd') DO	@set PWD=%%~fsi
@goto :EOF
