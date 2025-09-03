@echo off

SET zip="C:\Program Files\7-Zip\7z.exe"

echo Preparing CLEO for GTA III and GTA VC
FOR /F "USEBACKQ" %%F IN (`powershell -NoLogo -NoProfile -Command ^(Get-Item "bin\CLEO.asi"^).VersionInfo.FileVersion`) DO (SET fileVersion=%%F)
echo Detected version: %fileVersion%
SET outputFile=".\III.VC.CLEO_v%fileVersion%.zip"
if exist %outputFile% del %outputFile% /q

%zip% a -tzip %outputFile% ".\Changelog.md" -bb2 | findstr "+" 
%zip% rn %outputFile% "Changelog.md" "CLEO_Readme\Changelog.txt" -bso0

%zip% a -tzip %outputFile% ".\Readme.md" -bb2 | findstr "+" 
%zip% rn %outputFile% "Readme.md" "CLEO_Readme\Readme.txt" -bso0

%zip% a -tzip %outputFile% ".\bin\*" -bb2 -xr!*.tmp -xr!*.pdb -xr!*.ilk -xr!*.ipdb -xr!*.iobj -xr!*.db -xr!*.exp -xr!*.lib | findstr "+"

pause
