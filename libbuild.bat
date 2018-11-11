@echo off

del oppai.dll >nul 2>&1
del oppai.obj >nul 2>&1
del oppai.exp >nul 2>&1
cl ^
  /O2 /nologo /Gm- /GR- /EHsc /W4 ^
  /D_CRT_SECURE_NO_WARNINGS=1 /DNOMINMAX=1 ^
  /DOPPAI_EXPORT /D_WINDLL /D_USRDLL ^
  oppai.c ^
  /LD ^
  || EXIT /B 1
