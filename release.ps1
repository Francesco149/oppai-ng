# you must allow script execution by running
# 'Set-ExecutionPolicy RemoteSigned' in an admin powershell
# this requires vcvars to be already set (see vcvarsall17.ps1)
# 7zip is also required (choco install 7zip and add it to path)

$dir = Split-Path -Parent $MyInvocation.MyCommand.Definition

Push-Location "$dir"
git pull origin master -q

function Write-Header {
  param ([string]$Text)
  Write-Host $Text -Foreground Yellow -Background Black
}

function Header {
  param ([string]$Title)
  Write-Header ""
  Write-Header "##########################################################"
  Write-Header "> $Title"
}

cmd /c "build.bat"; if (-not $?) { exit $LastExitCode }
cmd /c "libbuild.bat"; if (-not $?) { exit $LastExitCode }
Header "Packaging"
$folder = "oppai-" + $(.\oppai.exe -version) + "-windows-"
$clout = & cl 2>&1 | %{ "$_" }
"$clout" -match "(Microsoft.*for )([a-z0-9\-_]+)" | Out-Null
if (-not $?) {
  exit $LastExitCode
}
$folder = $folder + $Matches[2]
mkdir $folder; if (-not $?) { exit $LastExitCode }
Copy-Item oppai.exe $folder; if (-not $?) { exit $LastExitCode }
Copy-Item oppai.dll $folder; if (-not $?) { exit $LastExitCode }
Copy-Item oppai.lib $folder; if (-not $?) { exit $LastExitCode }
git archive HEAD --prefix=src\ -o $folder\src.zip
if (-not $?) {
  exit $LastExitCode
}
Set-Location $folder; if (-not $?) { exit $LastExitCode }
&7z x src.zip; if (-not $?) { exit $LastExitCode }
Set-Location ..; if (-not $?) { exit $LastExitCode }

if (Test-Path "$folder.zip") {
  Remove-Item "$folder.zip"
}

&7z a "$folder.zip" $folder\oppai.exe $folder\oppai.dll $folder\oppai.lib `
  $folder\src
if (-not $?) {
  exit $LastExitCode
}

Header "Result:"
&7z l "$folder.zip"

Remove-Item $folder -Force -Recurse
Pop-Location
