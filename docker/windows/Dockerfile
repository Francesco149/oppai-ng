# escape=`

# I wanted to use nanoserver but I couldn't get visual c++ build
# tools to install. the nuget visual c++ build tools package isn't
# enough to get a working compiler and manually copying msvc
# defeats the purpose of using the container to automatically
# install deps.
# unfortunately windowsservercore is a 4GB image
FROM microsoft/windowsservercore

RUN @"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -InputFormat None -ExecutionPolicy Bypass -Command "iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))" && SET "PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin"

# wait for vs_installer.exe, vs_installerservice.exe
# or vs_installershell.exe because choco doesn't
RUN powershell -NoProfile -InputFormat None -Command `
    choco install git 7zip -y; `
    choco install visualcpp-build-tools `
        --version 15.0.26228.20170424 -y; `
    Write-Host 'Waiting for Visual C++ Build Tools to finish'; `
    Wait-Process -Name vs_installer

WORKDIR C:\oppai-ng
CMD powershell -ExecutionPolicy Bypass -Command `
        Get-Location; Get-ChildItem; `
        . .\vcvarsall17.ps1 x64; .\release.ps1; `
        . .\vcvarsall17.ps1 x86; .\release.ps1