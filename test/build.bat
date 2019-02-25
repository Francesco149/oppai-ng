@echo off

set flags="%*"
IF "%1"=="" (
  set flags=-DOPPAI_IMPLEMENTATION
)

del oppai_test.exe >nul 2>&1
del test.obj >nul 2>&1
cl  -D_CRT_SECURE_NO_WARNINGS=1 ^
  -DNOMINMAX=1 ^
  -O2 -nologo -MT -Gm- -GR- -EHsc -W4 ^
  %flags% ^
  test.c ^
  -Feoppai_test.exe ^
  || EXIT /B 1
