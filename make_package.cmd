@echo off

set CHROME_PATH="%HOME%\AppData\Local\Google\Chrome\Application"
set EXTENSION_DIR=%CD%\NC4GC
set EXTENSION_KEY_FILE=%CD%\NC4GC.pem

%CHROME_PATH%\chrome.exe --pack-extension=%EXTENSION_DIR% --pack-extension-key=%EXTENSION_KEY_FILE%