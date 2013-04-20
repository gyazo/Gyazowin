type
  WindowsNTVersion = (XP, Vista, VistaSp1, VistaSp2, Seven, SevenSp1, Eight);

const
  ntvXP       = $05010A28;
  ntvVista    = $06001770;
  ntvVistaSp1 = $06001771;
  ntvVistaSp2 = $06001772;
  ntvSeven    = $06011DB0; 
  ntvSevenSp1 = $06011DB1;
  ntvEight    = $060223F0;

function IsWindowsVersionOrLater(version: WindowsNTVersion): Boolean;
begin
  case version of
  XP: 
    Result := (GetWindowsVersion >= ntvXP);
  Vista: 
    Result := (GetWindowsVersion >= ntvVista);   
  VistaSp1: 
    Result := (GetWindowsVersion >= ntvVistaSp1);
  VistaSp2: 
    Result := (GetWindowsVersion >= ntvVistaSp2);
  Seven: 
    Result := (GetWindowsVersion >= ntvSeven);
  SevenSp1: 
    Result := (GetWindowsVersion >= ntvSevenSp1);
  Eight: 
    Result := (GetWindowsVersion >= ntvEight);    
  end;
end;

function IsVistaOrBefore: Boolean;
begin
  Result := not IsWindowsVersionOrLater(Seven); 
end;

function IsSevenOrLater: Boolean;
begin
  Result := IsWindowsVersionOrLater(Seven); 
end;

function IsSevenOrBefore: Boolean;
begin
  Result := not IsWindowsVersionOrLater(Eight); 
end;

function IsEightOrLater: Boolean;
begin
  Result := IsWindowsVersionOrLater(Eight); 
end;