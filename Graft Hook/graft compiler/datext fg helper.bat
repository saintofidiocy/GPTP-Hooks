@echo off
setlocal
if not "%~1"=="" goto decompile
choice /c CD /m "Press C to Compile txt files or D to Decompile FGP file."
if ERRORLEVEL ==2 goto decompile
if ERRORLEVEL ==1 goto compile
goto end

:compile
echo.
set /p unitc="Enter number of units.dat entries (blank for default values): "
set statc=%unitc%
::set /p upgdc="Number of upgrades: "
::set /p techc="Number of techs:    "
::set /p orderc="Number of orders:   "

if not defined unitc set unitc=250
if not defined statc set statc=228
::if not defined upgdc set upgdc=61
::if not defined techc set techc=44
::if not defined orderc set orderc=189

echo.
echo -- Merging Buttons ...
graftc -c -s -nb %unitc% -nu %statc% -ib Buttons.txt -ob Buttons.graft
echo.
echo -- Compiling Req Scripts ...
graftc -c -nb %unitc% -nu %statc% -np 255 -nt 255 -no 255 -iu Units.txt -ip Upgrades.txt -it Tech.txt -ir Research.txt -io Orders.txt -ou Units.graft -op Upgrades.graft -ot Tech.graft -or Research.graft -oo Orders.graft
goto end

:decompile
if "%~1"=="" goto selfgp
set fgp=%1
goto decompcont

:selfgp
echo.
set /p fgp="Enter FGP filename: "
set fgp="%fgp%"
echo.

:decompcont
if not defined fgp goto nofile
echo -- Decompiling ...
graftc -d -f %fgp% -nb 912 -nu 912 -np 255 -nt 255 -no 255 -ob Buttons.txt -ou Units.txt -op Upgrades.txt -ot Tech.txt -or Research.txt -oo Orders.txt
goto end

:nofile
echo No file specified.

:end
echo.
pause
endlocal
