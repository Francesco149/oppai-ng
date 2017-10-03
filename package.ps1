$dir = Split-Path -Parent $MyInvocation.MyCommand.Definition
Push-Location $dir
docker run --rm -v ${dir}:c:\oppai-ng oppai-ng:windows
Pop-Location