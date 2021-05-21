rem
rem File: stm32l0-upload.bat
rem
rem Function:
rem	Upload image (optionally plus bootloader) using DFU.
rem
rem Copyright & License:
rem	See accompanying license file
rem
rem Author:
rem	Terry Moore, MCCI	May 2021
rem
rem Arguments:
rem	stlink_upload [flags] {tools} {image} {base} {bootloader}
rem
rem	[flags]		-v for verbose, -nv for quiet; -vx / -nvx same, but exit cmd on error
rem	{tools}		full path with drive letter to STLINK tools
rem	{image}		path to image to download
rem	{base}		base address, 0x08000000 or 0x08005000
rem	{bootloader}	[optional] bootloader file
rem
rem	Script is currently very fragile and only intended to be
rem	called from Arduino IDE platform.txt.
rem

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

set tools=%1
rem Change forward slash to backslash in the tools dir name
set tools=%tools:/=\%
cd /d %tools%\tools\win

rem Change forward slash to backslash in the bin file name
set file=%2
set file=%file:/=\%
if "%2" == "" (
	echo Must provide image name
	exit %EXITFLAG% 1
)
if not exist %file% (
	echo can't find file: %file%
	exit %EXITFLAG% 1
)
if "%3" == "" (
	echo must provide base address
	exit %EXITFLAG% 1
)
set baseaddr=%3
if "%baseaddr%" == "" (
	echo must provide baseaddr
	exit %EXITFLAG% 1
)
set bootloader=%4
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
	set bootloaderflags=-P "%bootloader%" 0x08000000
) else if "%baseaddr%" == "0x08005000" (
	set OPTREPLACING=1
)

if %VERBOSE% GTR 0 (
	echo Program file:   %file%
	echo Path to tools:  %1
	echo Base address:   %baseaddr%
	if not "%bootloader%" == "-" (
	 echo Bootloader:     %bootloader%
	) else if %OPTREPLACING% GTR 0 (
	 echo Bootloader:     using existing bootloader
	) else (
	 echo Bootloader:     overwriting -- no bootloader
	)
)

rem: ------------- use STLINK CLI
rem: echo "" is mandatory to exit ST-Link_CLI tools w 3.0.0.0 when "-Run" option is used,
rem: it now wait Enter key press to quit.
echo "" | stlink\ST-LINK_CLI.exe -c SWD UR %bootloaderflags% -P %file% %baseaddr% -Rst -Run

if %ERRORLEVEL% GTR 0 (
	exit %EXITFLAG% %ERRORLEVEL%
)
