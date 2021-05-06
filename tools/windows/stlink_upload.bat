@echo off
rem
rem arguments:
rem	[ -v for verbose, -nv for quiet ]
rem	full path with drive letter to STLINK tools
rem	path to image to download
rem	base address
rem

set VERBOSE=0
if "%1" == "-v" (
	set VERBOSE=1
	shift
) else if "%1" == "-nv" (
	set VERBOSE=0
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
	goto :eof
)
if not exist %file% (
	echo can't find file: %file%
	goto :eof
)
if "%3" == "" (
	echo must provide base address
	goto :eof
)
set baseaddr=%3

if %VERBOSE% GTR 0 (
	echo Program file:   %file%
	echo Path to tools:  %1
	echo Base address:   %baseaddr%
)

rem: ------------- use STLINK CLI
rem: echo "" is mandatory to exit ST-Link_CLI tools w 3.0.0.0 when "-Run" option is used,
rem: it now wait Enter key press to quit.
echo "" | stlink\ST-LINK_CLI.exe -c SWD UR -P %file% %baseaddr% -Rst -Run
