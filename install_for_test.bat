@echo off
setlocal
rem Installs Insert Memo for the current user (no admin rights needed).
set "DEST=%LOCALAPPDATA%\Programs\Common\VST3"
pushd "%~dp0"
if not exist "Insert Memo.vst3" (
  echo [ERROR] "Insert Memo.vst3" was not found next to this file.
  popd
  pause
  exit /b 1
)
if not exist "%DEST%" mkdir "%DEST%"
if exist "%DEST%\Insert Memo.vst3" rmdir /s /q "%DEST%\Insert Memo.vst3"
xcopy /e /i /y /q "Insert Memo.vst3" "%DEST%\Insert Memo.vst3" >nul
if errorlevel 1 (
  echo [ERROR] Copy failed. Close the DAW and try again.
) else (
  echo Installed to: %DEST%\Insert Memo.vst3
  echo Rescan plug-ins in your DAW.
)
popd
pause
