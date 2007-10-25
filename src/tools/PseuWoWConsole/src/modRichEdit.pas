unit modRichEdit;

interface
uses SysUtils, Classes, StdCtrls, ComCtrls, Graphics, StrUtils, Windows;

procedure AddColouredLine(ARichEdit : TRichEdit; AText : String; AColor : TColor);
procedure AddColourToLine(ARichEdit : TRichEdit; AText : String);

function  HexToColor(sColor : String): TColor;

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

procedure AddColourToLine(ARichEdit : TRichEdit; AText : String);
var
  i : Integer;
  myColor : TColor;
  sTemp : String;
begin
  i := AnsiPos('|c',LowerCase(AText));

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
        AddColouredLine(ARichEdit, AText, myColor);
{
        SelStart := Length(AText) - i;
        SelAttributes.Color := myColor;
        SelAttributes.Size := 8;
}
      end;

      i := AnsiPos('|c',LowerCase(AText));
    end
    else
      i := 0;
  end;

  ARichEdit.Lines.Add(AText);
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

end.
