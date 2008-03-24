unit modSCPUtils;

interface
  uses Sysutils, Classes, StrUtils, Dialogs;

  function GetHeader(AString: String): String;
  function EvaluateStringAsHeader(AString, AHeader : String): Boolean;
  function EvaluateProperty(ABuffer, AProp : String; var sValue : String): Boolean;
  function EvaluatePropertyAsInt(ABuffer, AProp : String; var iValue : Integer): Boolean;
  function UpdateProperty(AProperty, Value: String; AList : TStringList ; AddProperty : Boolean = true; AllowMultiples : Boolean = false; PropUpperCase : Boolean = True): Boolean;
  function DeleteProperty(AProperty: String; AList : TStringList; Multiples : Boolean = false): Boolean;
  function StripCommentsFromString(AString : String): String;
  function GetNumbersFromString(AString, ASeparator, AHeader :String; AElements : Integer; var ALeftOver : String; AllowBlank : Boolean = False; DoTrim : Boolean = True): TStringList;
  function GetCustomArray(AString : String; AStringListofArray: TStrings; AElementIndex, AElementCount : Integer): String;
  function GetRandom(AString: String; GetValuesInBetween : Boolean = True): String;
  function GetString(AMin, AMax: String; Seperator : String = ' '): String; overload;
  function GetString(AStringList : TStringList; Seperator: String = ' '): String; overload;
  function MakeList(AStringList : TStringList; AProp : String): TStringList;
{
  Moved into TSCPItem
  function GetList(AStringList : TStringList; AProp : String): TStringList;
}
  function AddProp(AProp, AValue : String; AList : TStringList): Boolean; overload;
  function AddProp(AStrings: TStringList; AList : TStringList): Boolean; overload;
  function AddToList(AList : TStringList; AValue : String): Boolean;
  function GetFloat(AString : String): Extended;
  function CountCharOccurences(const S: string; const ch: string): Integer;
  function CountSubStringOccurences(const subtext: string; Text: string): Integer;
  function CountPropOccurences(AProp : String; AList : TStringList): Integer;

implementation

function GetHeader(AString: String): String;
var
  i : Integer;
begin
  Result := '';

  //[item or [creature etc.
  if AnsiPos('[', AString) <> 0 then
  begin
    i := AnsiPos(' ', AString);
    if i <> 0 then
    begin
      Result := Copy(AString,0,i-1);
      Exit;
    end;
  end;

  i := AnsiPos('=', AString);
  if i <> 0 then
  begin
    Result := Copy(AString,0,i-1);
    Exit;
  end;

end;

function EvaluateStringAsHeader(AString, AHeader: String): Boolean;
begin
  Result := False;

  //Remove the = for simpler comparison
  if AnsiContainsText(AHeader,'=') then
    AHeader := AnsiReplaceStr(AHeader,'=','');

  AHeader := Trim(AHeader);
  AString := Trim(AString);

  AHeader := UpperCase(AHeader);
  AString := UpperCase(AString);

  if LeftStr(AString,(Length(AHeader))) = AHeader then
  begin
    Result := True;
  end;

  //For my Number Hack!
  if (LeftStr(AHeader,1) = '~') and (LeftStr(AString,1) = '~') then
    Result := True;

end;

function EvaluateProperty(ABuffer, AProp: String;
  var sValue: String): Boolean;
var
  i : Integer;
begin
  sValue := '';

  i := Length(AProp);
  Result := False;

  if (UpperCase(Copy(ABuffer, 0, i))) = (UpperCase(AProp)) then
  begin
    Result := True;
    sValue := Trim(RightStr(ABuffer, (Length(ABuffer) - i)));
  end;
end;

function EvaluatePropertyAsInt(ABuffer, AProp : String; var iValue : Integer): Boolean;
var
  sTemp : String;
begin
  Result := False;

  if (EvaluateProperty(ABuffer, AProp, sTemp)) and (TryStrToInt(sTemp, iValue)) then
  begin
    Result := True;
  end;
end;

function UpdateProperty(AProperty, Value: String; AList : TStringList ; AddProperty : Boolean = true; AllowMultiples : Boolean = false; PropUpperCase : Boolean = True): Boolean;
var
  i, j, L1, L2, iFound : integer;
  sTemp, sValue : String;
  bFound : Boolean;
  slFound : TStringList;
begin
  try
    slFound := TStringList.Create;
    slFound.Clear;

    //For Neatness
    if (PropUpperCase = True) then
      AProperty := Trim(UpperCase(AProperty))
    else
      AProperty := Trim(AProperty);

    Value := Trim(Value);

    if AProperty = '' then
      Exit;

    for i := 0 to AList.Count - 1 do
    begin
      if EvaluateProperty(AList.Strings[i],AProperty,sTemp) then
      begin
        slFound.Add(IntToStr(i));
        j := i;
        bFound := True;
        if not(AllowMultiples) then
          Break;
      end;
    end;

    //delte the property
    if Value = '' then
    begin
      AList.Delete(j);
      Exit;
    end;

    if not(bFound) and not(AddProperty) then
      Exit;

    sValue := AProperty+'='+Value;

    if not(bFound) then //not found then add it
    begin
      AList.Add(sValue);
    end
    else
    begin //found then update it
      if not(AllowMultiples) then // Only want a single instance of this boy
        AList.Strings[j] := sValue
      else
      begin // as in EQUIP=0 or EQUIP=1 or Equip=2

        for i := 0 to slFound.Count - 1 do
        begin
          iFound := StrToInt(slFound[i]);
          L1 := (AnsiPos(' ',AList[iFound])) - 1;
          L2 := (AnsiPos(' ',sValue)) - 1;

          if L1 <= 0 then
          begin
            AList.Add(sValue);
            Exit;
          end;

          if (UpperCase(Copy(AList[iFound],0, L1))) = (UpperCase(Copy(sValue,0, L2))) then
          begin
            AList[iFound] := sValue;
            Exit;
          end;

        end;
        //ok we looked but it isn't there so add it
        AList.Add(sValue);
      end;
    end;
  finally
    slFound.Free;
  end;
end;

function DeleteProperty(AProperty: String; AList : TStringList;
  Multiples: Boolean): Boolean;
var
  i : integer;
  sTemp : string;
begin
  for i := AList.Count - 1 downto 0 do
  begin
    if EvaluateProperty(AList[i],AProperty, sTemp) then
    begin
      AList.Delete(i);
      if not(Multiples) then
        Break;
    end;
  end;
end;

function GetNumbersFromString(AString, ASeparator, AHeader :String; AElements : Integer; var ALeftOver : String; AllowBlank : Boolean = False; DoTrim : Boolean = True): TStringList;
var
  iLength, iSubLength, iPos, iIndex, iLastCheck : Integer;
  sTemp : String;
  rArray : Array of Real;
  oList : TStringList;
begin
  if Result = nil then
    Result := TStringList.Create
  else
    Result.Clear;

  //Hopefully this wont fux things up
  AString := UTF8Decode(AString);


  //A lazy way to tell it to look for everything
  if AElements = 0 then
    AElements := 100;

  //Check for Delim
  if ASeparator = '[SPACE]' then
    ASeparator := ' ';

  if ASeparator = '' then
    ASeparator := ' ';

  if AllowBlank then
    AElements := CountCharOccurences(AString,  ASeparator);

  if AHeader <> '' then
    AString := AnsiReplaceText((AString), (AHeader), '');
  if AHeader <> '' then
    AString := AnsiReplaceText(AString, '=' ,'');
  if AHeader <> '' then
    AString := AnsiReplaceText(AString, '~','');
  if DoTrim then
    AString := Trim(AString);

  SetLength(rArray, AElements);

  for iIndex := 0 to AElements - 1 do
  begin
    iPos := AnsiPos(ASeparator, AString);

    if AElements > 1 then //if we have mulitple elements
    begin
      if iIndex < (AElements - 1) then
      begin
        if iPos > 0 then
          sTemp := LeftStr(AString, iPos - 1)
        else
          sTemp := AString;
      end
      else
        sTemp := AString;
    end
    else
    begin //if we only have one element
      if iPos > 0 then
        sTemp := LeftStr(AString, iPos - 1)
      else
        sTemp := AString;
    end;

    //If we have processed all our elements make sure there aren't additional ones
    if iIndex = (AElements - 1) then
    begin
      if ASeparator = '[SPACE]' then
        iLastCheck := AnsiPos(' ', sTemp)
      else
        iLastCheck := AnsiPos(ASeparator, sTemp);

      if iLastCheck > 0 then
      begin
        //Removing Excess Elements from sTemp
        sTemp := LeftStr(AString, iLastCheck - 1)
      end;
    end;

    if (sTemp = '') then
    begin
      if not(AllowBlank) then
        Continue
      else
        Result.Add(sTemp);
    end
    else
      Result.Add(sTemp);

    iLength := Length(AString);
    iSubLength := Length(sTemp);

    if ASeparator = '[SPACE]' then
      AString := RightStr(AString, (iLength - (iSubLength + 1 )))
    else
      AString := RightStr(AString, (iLength - (iSubLength+(Length(ASeparator)))));

    if DoTrim then
      AString := Trim(AString);

  end;

  ALeftOver := AString;

end;

function GetCustomArray(AString : String; AStringListofArray: TStrings; AElementIndex, AElementCount : Integer): String;
var
  iIndex, iPos : Integer;
  oStringList : TStringList;
  sFNM : string;
begin
  oStringList := TStringList.Create;

  for iIndex := 0 to (AStringListofArray.Count - 1) do
  begin
    if LeftStr(AStringListofArray.Strings[iIndex], (Length(AString)+1)) = AString + ' ' then
    begin
      AString := AnsiReplaceStr(AStringListofArray.Strings[iIndex][iIndex], AString + ' ', '');
      oStringList := GetNumbersFromString(AString, ' ', '', AElementCount, sFNM);
      Result := oStringList.Strings[AElementIndex - 1];
      Exit;
    end
  end;


  oStringList.Free;
end;


function GetRandom(AString: String; GetValuesInBetween : Boolean = True): String;
var
  iPos, iTotal, iMin, iMax, iLast, iRange, iRand : Integer;
  sBuffer : String;
  sList : TStringList;
begin
  try
    sList := TStringList.Create;

    AString := AnsiReplaceText(AString,'..',' ');
    AString := AnsiReplaceStr(AString,',',' ');

    AString := Trim(AString);

    iPos := AnsiPos(' ',AString);

    while iPos <> 0 do
    begin

      sBuffer := (Copy(AString,0,(iPos-1)));

      sList.Add(sBuffer);
      AString := (Copy(AString,iPos,(Length(AString))));
      AString := Trim(AString);
      iPos := AnsiPos(' ',AString);
    end;

    //Get the Last One
    sList.Add(AString);

    if sList.Count > 1 then
    begin
      if GetValuesInBetween then //calc values between first and last
      begin
        iMin := StrToInt(sList.Strings[0]);
        iLast := sList.Count - 1;
        iMax := StrToInt(sList.Strings[iLast]);
        iRange := iMax - iMin;

        if iRange <= 0 then
        begin
          Result := IntToStr(iMin);
          Exit;
        end;

        iRand := Round(Random(iRange));

        Result := IntToStr(iMin + iRand);

      end
      else
      begin // select a random value in the list
        iRand := Round(Random(sList.Count-1));
        sBuffer := sList.Strings[iRand];
        Result := sBuffer;
      end;
    end
    else
    begin
      sBuffer := sList.Strings[0];
      result := sBuffer;
    end;

  finally
    sList.Free;
  end;
end;

function StripCommentsFromString(AString : String): String;
var
  iComment : Integer;
begin

  if AString <> '' then
  begin
    iComment := AnsiPos('/',AString);

    //Remove comments
    if iComment > 0 then
      AString := Copy(AString,0,(iComment-1));

  end;

  Result := Trim(AString);

end;

function GetString(AStringList: TStringList;
  Seperator: String = ' '): String;
var
  i : integer;
begin
  Result := '';
  for i := 0 to AStringList.Count - 1 do
  begin
    Result := Result + AStringList[i];
    if i < (AStringList.Count - 1) then
      Result := Result + Seperator;
  end;
end;

function GetString(AMin, AMax, Seperator: String): String;
begin
  Result := '';

  if (AMin <> '') and (AMin <> '0') and (AMax <> '')  and (AMax <> '0') and (AMin <> AMax) then
  begin
    Result := AMin + Seperator + AMax;
  end
  else
  //Induvidual
  begin
    if (AMin <> '') and (AMin <> '0') then
      Result := AMin;
    if (AMax <> '') and (AMax <> '0') then
      Result := AMax;
  end;
end;

function MakeList(AStringList: TStringList;
  AProp: String): TStringList;
var
  i : Integer;
begin
  Result := TStringList.Create;

  for i := 0 to AStringList.Count - 1 do
  begin
    Result.Add(AProp+'='+AStringList[i]);
  end;
end;

//Moved into TSCPItem
{
function GetList(AStringList : TStringList; AProp : String): TStringList;
var
  i : Integer;
  sBuffer : String;
begin
  Result := TStringList.Create;

  for i := 0 to AStringList.Count - 1 do
  begin
    if EvaluateProperty(AStringList[i], AProp, sBuffer) then
      Result.Add(sBuffer)
  end;

  if Result.Count = 0 then
  begin
    FreeAndNil(Result);
  end;

  Result.Sort;
end;
}
function AddProp(AProp, AValue: String; AList : TStringList): Boolean;
var
  i : integer;
begin
  Result := False;

  if Trim(AValue) = '' then
    Exit;

  if AnsiPos('=',AProp) = -1 then
    AProp := AProp +'=';

  UpdateProperty(AProp,AValue,AList,true,false,false);

  Result := True;
end;

function AddProp(AStrings: TStringList; AList : TStringList): Boolean;
begin
  if AStrings.GetText <> '' then
    AList.AddStrings(AStrings);
end;

function AddToList(AList: TStringList;
  AValue: String): Boolean;
begin
  Result := False;

  if (AValue = '') or (AValue = '0') then
    Exit;

  try
    if not(Assigned(AList)) then
      AList := TStringList.Create;
    if AList.IndexOf(AValue) = -1 then
      AList.Add(AValue);
  except
    Result := False;
    //Raise the error
    Exit;
  end;
  Result := True;
end;

function GetFloat(AString : String): Extended;
var
  Code: Integer;
begin
  Val(AString, Result, Code);

  if Code <> 0 then
  begin
    MessageDlg('Error converting Float in '+AString+' position: ' + IntToStr(Code), mtWarning, [mbOk], 0);
    Result := 0;
  end;
end;

function CountCharOccurences(const S: string; const ch: string): Integer;
var
  buf: string;
begin
  buf := S;
  Result := 0;
  {while Pos finds a blank}
  while (Pos(ch, buf) > 0) do
    begin
      {copy the substrings before the blank in to Result}
      Result := Result + 1;
      buf := Copy(buf, Pos(ch, buf) + 1, Length(buf) - Pos(ch, buf));
    end;
end;

//http://www.delphitricks.com/source-code/strings/count_the_number_of_occurrences_of_a_substring_within_a_string.html
function CountSubStringOccurences(const subtext: string; Text: string): Integer;
begin
  if (Length(subtext) = 0) or (Length(Text) = 0) or (Pos(subtext, Text) = 0) then
    Result := 0
  else
    Result := (Length(Text) - Length(StringReplace(Text, subtext, '', [rfReplaceAll]))) div
      Length(subtext);
end;

function CountPropOccurences(AProp : String; AList : TStringList): Integer;
var
 i : Integer;
begin
  Result := 0;

  AProp := AnsiReplaceText(AProp, '=', '');

  AList.CaseSensitive := False;

  for i := 0 to AList.Count - 1 do
  begin
    if AList.Names[i] = AProp then
      Result := Result + 1;
  end;
end;

end.
