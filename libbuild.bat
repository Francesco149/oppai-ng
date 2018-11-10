@echo off

del oppai.dll >nul 2>&1
del oppai.obj >nul 2>&1
cl  -D_CRT_SECURE_NO_WARNINGS=1 ^
  -DNOMINMAX=1 ^
  -O2 -nologo -LD -MD -Gm- -GR- -EHsc -W4 ^
  oppai.c ^
  -Feoppai.dll ^
  || EXIT /B 1
