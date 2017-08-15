@echo off

del oppai.exe
del oppai.obj
cl  -D_CRT_SECURE_NO_WARNINGS=1 ^
    -DNOMINMAX=1 ^
    -O3 ^
    -nologo -MT -Gm- -GR- -EHsc -W4 ^
    main.c ^
    -Feoppai.exe
