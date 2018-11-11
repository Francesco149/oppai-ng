@echo off

del oppai.exe >nul 2>&1
del main.obj >nul 2>&1
cl  -D_CRT_SECURE_NO_WARNINGS=1 ^
  -DNOMINMAX=1 ^
  -O2 -nologo -MT -Gm- -GR- -EHsc -W4 ^
  main.c ^
  -Feoppai.exe ^
  || EXIT /B 1
