@echo off
setlocal enableextensions
set TERM=
cd /d "%~dp0bin" && .\bash --login -i
