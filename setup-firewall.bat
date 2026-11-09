@echo off
:: Oway - allows LAN games through Windows Firewall
::
:: Usage:
::   setup-firewall.bat          adds the rule (run on the computer that HOSTS the game)
::   setup-firewall.bat remove   removes the rule again
::
:: The rule opens only TCP port 52025 (DEFAULT_PORT in BLL/include/Network.h)
:: and only on private networks (home / school), not on public Wi-Fi.

setlocal
set "RULE_NAME=Oway LAN game"
set "PORT=52025"
set "ACTION=%~1"

:: Changing the firewall needs administrator rights - ask for them if we don't have them
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Asking for administrator rights...
    if "%ACTION%"=="" (
        powershell -NoProfile -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
    ) else (
        powershell -NoProfile -Command "Start-Process -FilePath '%~f0' -ArgumentList '%ACTION%' -Verb RunAs"
    )
    exit /b
)

if /i "%ACTION%"=="remove" goto remove

:add
:: An old rule with the same name is removed first, so running the script twice doesn't make two rules
netsh advfirewall firewall delete rule name="%RULE_NAME%" >nul 2>&1
netsh advfirewall firewall add rule name="%RULE_NAME%" dir=in action=allow protocol=TCP localport=%PORT% profile=private >nul

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] The firewall rule could not be added.
    goto end
)

echo.
echo [OK] Oway can now receive LAN games on TCP port %PORT%.
echo.
echo If the other player still can't join:
echo   - both computers must be on the same network
echo   - Windows must see this network as "Private"
echo     (Settings ^> Network ^& Internet ^> your network ^> Network profile type)
goto end

:remove
netsh advfirewall firewall delete rule name="%RULE_NAME%" >nul 2>&1

if %errorlevel% neq 0 (
    echo.
    echo [INFO] There was no Oway firewall rule to remove.
) else (
    echo.
    echo [OK] The Oway firewall rule was removed.
)

:end
echo.
pause
endlocal
