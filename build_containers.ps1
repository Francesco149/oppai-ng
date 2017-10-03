$dir = Split-Path -Parent $MyInvocation.MyCommand.Definition
Push-Location "$dir"
$d = "docker/windows"
Copy-Item $d/Dockerfile .
docker build -t "oppai-ng:windows" .
Remove-Item Dockerfile
Pop-Location