# you must allow script execution by running
# 'Set-ExecutionPolicy RemoteSigned' in an admin powershell
# this requires vcvars to be already set (see vcvarsall17.ps1)
# 7zip is also required (choco install 7zip and add it to path)

$dir = Split-Path -Parent $MyInvocation.MyCommand.Definition

pushd "$dir"
git pull origin master

Write-Host ""
Write-Host "########################" -Foreground Yellow -Background Black
Write-Host "> Compiling and stripping" -Foreground Yellow -Background Black
cmd /c "build.bat"

Write-Host ""
Write-Host "########################" -Foreground Yellow -Background Black
Write-Host "> Packaging" -Foreground Yellow -Background Black
$folder = "oppai-" + $(.\oppai.exe -version) + "-windows-"
$clout = &cl 2>&1
"$clout" -match "^(Microsoft.*for )([a-z0-9\-_]+)" | Out-Null
$folder = $folder + $Matches[2]
mkdir $folder
Move-Item oppai.exe $folder
git archive HEAD --prefix=src\ -o $folder\src.zip
cd $folder
&7z x src.zip
cd ..

if (Test-Path "$folder.zip") {
    Remove-Item "$folder.zip"
}

&7z a "$folder.zip" $folder\oppai.exe $folder\src

Write-Host ""
Write-Host "########################" -Foreground Yellow -Background Black
Write-Host "> Result:" -Foreground Yellow -Background Black
&7z l "$folder.zip"

Remove-Item $folder -Force -Recurse
popd