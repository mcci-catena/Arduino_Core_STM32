@echo off

rem
rem arguments:
rem	[ -v for verbose, -nv for quiet; -vx / -nvx same, but exit cmd on error ]
rem	full path with drive letter to STLINK tools
rem	path to image to download
rem	base address
rem	[optional] bootloader file

cd /d %~dp0

set VERBOSE=0
set EXITFLAG=/b
if "%1" == "-v" (
	set VERBOSE=1
	shift
) else if "%1" == "-nv" (
	set VERBOSE=0
	shift
) else if "%1" == "-vx" (
	set VERBOSE=1
	set EXITFLAG=
	shift
) else if "%1" == "-nvx" (
	set VERBOSE=0
	set EXITFLAG=
	shift
)

set VIDPID=
if not "%1" == "-" (
    if not "%2" == "-" (
        set VIDPID=%1:%2,
    )
) else if not "%2" == "-" (
    echo First two args are vid/pid and both must be - if either is
    exit %EXITFLAG% 1
)

rem Change forward slash to backslash in the bin file name
set file=%3
set file=%file:/=\%
if "%3" == "" (
	echo Must provide image name
	exit %EXITFLAG% 1
)
if not exist %file% (
	echo can't find file: %file%
	exit %EXITFLAG% 1
)
if "%4" == "" (
	echo must provide base address
	exit %EXITFLAG% 1
)
set baseaddr=%4
if "%baseaddr%" == "" (
	echo must provide baseaddr as parameter 4
	exit %EXITFLAG% 1
)
set bootloader=%5
if "%bootloader%" == "" (
	set bootloader=-
)
set bootloader=%bootloader:/=\%

if not "%bootloader%" == "-" (
	if not exist %bootloader% (
		echo can't find bootloader %bootloader%
		exit %EXITFLAG% 1
	)
)
set bootloaderflags=
set OPTREPLACING=0
if not "%bootloader%" == "-" (
	rem make sure load address is right
	if not "%baseaddr%" == "0x08005000" (
		echo can't install bootloader if you select 'basic/no bootloader' in Tools/Trusted boot
		exit 1
	)
	set OPTREPLACING=1
) else if "%baseaddr%" == "0x08005000" (
	set OPTREPLACING=1
)

if %VERBOSE% GTR 0 (
	echo Program file:   %file%
	echo Board VID:PID:  %VIDPID%
	echo Base address:   %baseaddr%
	if not "%bootloader%" == "-" (
	 echo Bootloader:     %bootloader%
	) else if %OPTREPLACING% GTR 0 (
	 echo Bootloader:     using existing bootloader
	) else (
	 echo Bootloader:     overwriting
	)
)

set count=1

:wait
    ".\dfu-util.exe" -l -d 0483:df11 | findstr "Found" >NUL 2>&1
    if %ERRORLEVEL% == 0 (
        goto :download
    ) else (
        if %count% gtr 10 (
            echo device not found
            exit %EXITFLAG% 1
        )
        echo %count%
        set /A count+=1
        @rem to wait for 1 second, you have to ping twice...
        ping -n 2 -w 1000 127.0.0.1 >NUL
        goto :wait
    )

:download
    if not "%bootloader%" == "-" (
	    "%.\dfu-util.exe" -d %VIDPID%0x0483:0xdf11 -a 0 -s 0x08000000 -D "%bootloader%"
        if %ERRORLEVEL% GTR 0 (
            exit %EXITFLAG% %ERRORLEVEL%
        )
    )
	".\dfu-util.exe" -d %VIDPID%0x0483:0xdf11 -a 0 -s %baseaddr%:leave -D "%file%"
    if %ERRORLEVEL% GTR 0 (
        exit %EXITFLAG% %ERRORLEVEL%
    )

:break
