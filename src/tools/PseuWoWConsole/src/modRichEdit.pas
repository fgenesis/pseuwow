unit modRichEdit;

interface
uses SysUtils, Classes, StdCtrls, ComCtrls, Graphics, StrUtils, Windows;

procedure AddColouredLine(ARichEdit : TRichEdit; AText : String; AColor : TColor);
procedure AddColourToLine(ARichEdit : TRichEdit; AText : String; ADefautColor : TColor = clLime);

function  HexToColor(sColor : String): TColor;
function  AddHilightedItem(AString, AHText : String): string;

implementation

procedure AddColouredLine(ARichEdit : TRichEdit; AText : String; AColor : TColor);
begin
  with ARichEdit do
  begin
    SelStart := Length(Text);
    SelAttributes.Color :=  AColor;
    SelAttributes.Size := 8;
    Lines.Add(AText);
  end;
end;

procedure AddColourToLine(ARichEdit : TRichEdit; AText : String; ADefautColor : TColor);
var
  i : Integer;
  myColor : TColor;
  sTemp : String;
begin
  myColor := ADefautColor;

  i := AnsiPos('|c',LowerCase(AText));

  //TODO read multicolured lines

  while i <> 0 do
  begin
    if UpperCase(Copy(AText, i, 4)) = '|CFF' then
    begin
      with ARichEdit do
      begin
        //Get the color - |cffFF6600
        sTemp := Copy(AText,i, 10 );
        sTemp := AnsiReplaceText(sTemp,'|CFF','');

        myColor := HexToColor(sTemp);
        AText := AnsiReplaceText(AText,'|CFF'+sTemp,'');
{
        SelStart := Length(AText) - i;
        SelAttributes.Color := myColor;
        SelAttributes.Size := 8;
}
      end;

      i := AnsiPos('|c',LowerCase(AText));
    end
    else
      break;
  end;

  AddColouredLine(ARichEdit, AText, myColor);
end;

function  HexToColor(sColor : String): TColor;
begin
  sColor := UpperCase(sColor);
  Result := RGB(
    StrToInt('$'+Copy(sColor,1,2)),
    StrToInt('$'+Copy(sColor,3,2)),
    StrToInt('$'+Copy(sColor,4,2))
    );
end;

function  AddHilightedItem(AString, AHText : String): string;
var
  iPos, iPos2 : Integer;
begin
  if AnsiContainsText(AString, '|H'+AHText) then
  begin
    iPos := AnsiPos('|H' + AHText, AString);
    iPos2 := AnsiPos(']', AString);

    Result := Copy(AString, 0, iPos - 1);

    iPos := AnsiPos('[', AString);

    Result := Result + Copy(AString, iPos, iPos2);
    Result := AnsiReplaceText(Result, '|h','');
  end;

end;


end.
