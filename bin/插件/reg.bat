@echo off
rem	Check that the user	properly called	this script.

rem	设置APPDIR环境变量为当前服务器程序运行目录
call :compute_pwd
set	APPDIR=%~dp0%
echo 设置工作目录成功

echo "开始注册 screen-capture-recorder插件"

regsvr32 %APPDIR%screen-capture-recorder.dll
regsvr32 %APPDIR%audio_sniffer.dll

echo "注册完成"

:compute_pwd
@FOR /F	"tokens=*" %%i in ('cd') DO	@set PWD=%%~fsi
@goto :EOF
