@echo off

del oppai.dll >nul 2>&1
del oppai.obj >nul 2>&1
del oppai.exp >nul 2>&1
cl -c -D_CRT_SECURE_NO_WARNINGS=1 ^
  -DNOMINMAX=1 ^
  -O2 -nologo -LD -MT -Gm- -GR- -EHsc -W4 ^
  -DOPPAI_EXPORT ^
  -D_WINDLL -D_USRDLL ^
  oppai.c ^
  || EXIT /B 1
link -nologo -DLL oppai.obj || EXIT /B 1
