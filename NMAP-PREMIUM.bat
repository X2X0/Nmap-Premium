@echo off
setlocal enabledelayedexpansion

call :print_banner

set "target=example.com"
set "ports=1-1000"
set "num_threads=100"
set "verbose=1"
set "timeout=1"

call :get_ports_list

call :port_scan

exit /b

:print_banner
echo.
echo     ███╗   ██╗███╗   ███╗ █████╗ ██████╗     ██████╗ ██████╗ ███████╗███╗   ███╗██╗██╗   ██╗███╗   ███╗
echo     ████╗  ██║████╗ ████║██╔══██╗██╔══██╗    ██╔══██╗██╔══██╗██╔════╝████╗ ████║██║██║   ██║████╗ ████║
echo     ██╔██╗ ██║██╔████╔██║███████║██████╔╝    ██████╔╝██████╔╝█████╗  ██╔████╔██║██║██║   ██║██╔████╔██║
echo     ██║╚██╗██║██║╚██╔╝██║██╔══██║██╔═══╝     ██╔═══╝ ██╔══██╗██╔══╝  ██║╚██╔╝██║██║██║   ██║██║╚██╔╝██║
echo     ██║ ╚████║██║ ╚═╝ ██║██║  ██║██║         ██║     ██║  ██║███████╗██║ ╚═╝ ██║██║╚██████╔╝██║ ╚═╝ ██║
echo     ╚═╝  ╚═══╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝         ╚═╝     ╚═╝  ╚═╝╚══════╝╚═╝     ╚═╝╚═╝ ╚═════╝ ╚═╝     ╚═╝
echo.
echo     Advanced Port Scanner [Premium Version]
echo     Developed in Batch |  https://github.com/X2X0/Nmap-Premium
echo     ================================================================
exit /b

:get_ports_list
set "ports_to_scan="
for %%a in (%ports%) do (
    for /f "tokens=1-2 delims=-" %%b in ("%%a") do (
        for /l %%c in (%%b,1,%%d) do (
            set "ports_to_scan=!ports_to_scan! %%c"
        )
    )
)
exit /b

:port_scan
echo.
echo [*] Starting scan on %target%
echo [*] Scanning %ports_to_scan% ports with %num_threads% threads
echo [*] Start time: %time%
for %%p in (%ports_to_scan%) do (
    call :scan_port %%p
)
exit /b

:scan_port
set "port=%1"
echo Scanning port %port%...
powershell -Command "Test-NetConnection -ComputerName %target% -Port %port%"
exit /b
