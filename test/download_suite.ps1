# you must allow script execution by running
# 'Set-ExecutionPolicy RemoteSigned' in an admin powershell
# 7zip is also required (choco install 7zip and add it to path)

$url = Get-Content .\suite_url -Raw
$dir = Split-Path -Parent $MyInvocation.MyCommand.Definition
Push-Location "$dir"

if ((Test-Path .\test_suite) -and (Get-ChildItem .\test_suite | Measure-Object).Count -gt 0) {
  Write-Host "using existing test_suite"
} else {
  # my windows 7 install doesn't support Tls3
  [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
  (New-Object System.Net.WebClient).DownloadFile($url, "$dir\test_suite.tar.gz")
  &7z x .\test_suite.tar.gz
  &7z x .\test_suite.tar
}

Pop-Location
