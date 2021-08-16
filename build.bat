@echo off

REM ***************************** edit this with the correct path to vcvarsall.bat ******************************

set vcvarsallpath="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"

REM ******************************************************************************************************************

if exist deps\ goto skipdownload
echo Dependencies not found. Press any key to attempt download.
pause
mkdir deps 2>NUL
mkdir deps\gmod 2>NUL
mkdir deps\sranipal 2>NUL
pushd deps\gmod
powershell -command [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest https://github.com/Facepunch/gmod-module-base/archive/15bf18f369a41ac3d4eba29ee0679f386ec628b7.zip -Out tmp.zip; Expand-Archive tmp.zip -Force; Move-Item tmp\gmod-module-base-15bf18f369a41ac3d4eba29ee0679f386ec628b7\include\GarrysMod\Lua\* -Force; Remove-Item tmp.zip; Remove-Item tmp -Recurse;
popd
echo Download complete (if there are no errors above).
pause
:skipdownload

if not exist deps\sranipal\SRanipal.lib (
    echo.
    echo SRanipal dependencies missing.
    echo Please download the SRanipal SDK via the Vive website and copy all of the .h and .lib files directly into the deps\sranipal folder
    echo.
    pause
    exit
)

set CompilerFlags= -MT -nologo -Oi -O2 -W3 /wd4996 /I..\..\..\..\..\deps
set LinkerFlags= -INCREMENTAL:NO -opt:ref /LIBPATH:..\..\..\..\..\deps\sranipal /DLL
	
pushd bin\GarrysMod\garrysmod\lua\bin
call %vcvarsallpath% x64
cl %CompilerFlags% ..\..\..\..\..\src\source.cpp /link %LinkerFlags% /out:gmcl_sranipal_win64.dll
del gmcl_sranipal_win64.exp
del gmcl_sranipal_win64.lib
del source.obj
popd

pause