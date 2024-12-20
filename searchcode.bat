@echo off
:search
cls
set /p searchString=Enter the string to search for: 

if "%searchString%"=="" goto search

echo Searching for "%searchString%"...

findstr /s /m "%searchString%" *.*

set /p continueSearch=Do you want to continue searching? (Y/N): 

if /i "%continueSearch%"=="Y" goto search
if "%continueSearch%"=="" goto search

pause