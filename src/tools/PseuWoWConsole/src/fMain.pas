unit fMain;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, RedirectConsole, ExtCtrls, IniFiles, ScktComp, JvComponentBase,
  JvTrayIcon, ComCtrls, modRichEdit, StrUtils, ImgList, modSCPUtils;

type
  TfrmMain = class(TForm)
    pnlTop: TPanel;
    txtExe: TEdit;
    btnRun: TButton;
    btnExit: TButton;
    servRemote: TServerSocket;
    timerStart: TTimer;
    clientSock: TClientSocket;
    TrayIcon: TJvTrayIcon;
    Console: TRichEdit;
    imgList: TImageList;
    pnlBottom: TPanel;
    grpCmd: TGroupBox;
    comCommand: TComboBox;
    pnlSessionTop: TPanel;
    cbexIcon: TComboBoxEx;
    txtChar: TStaticText;
    procedure btnRunClick(Sender: TObject);
    procedure btnExitClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormCloseQuery(Sender: TObject; var CanClose: Boolean);
    procedure timerStartTimer(Sender: TObject);
    procedure servRemoteClientRead(Sender: TObject;
      Socket: TCustomWinSocket);
    procedure FormDestroy(Sender: TObject);
    procedure clientSockConnect(Sender: TObject; Socket: TCustomWinSocket);
    procedure clientSockError(Sender: TObject; Socket: TCustomWinSocket;
      ErrorEvent: TErrorEvent; var ErrorCode: Integer);
    procedure ConsoleResizeRequest(Sender: TObject; Rect: TRect);
    procedure comCommandKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure clientSockConnecting(Sender: TObject;
      Socket: TCustomWinSocket);
    procedure cbexIconChange(Sender: TObject);
  private
    { Private declarations }
    App : String;
    Running : Boolean;
    Ready : Boolean;

    function ConsoleCommand(AString : String):Boolean;

    procedure LoadSettings;
    procedure SetupIcons;
    procedure SetIcon(AIndex : Integer; AUpdateINI : Boolean = True);
    procedure LoadPseuSettings(AConFile : string);

    procedure ShutDown;
    procedure Execute(AFile: String);
    procedure Launch;
    procedure Log(AText: String; Color : TColor = clAqua);
    procedure WriteFromPseWow(AString : String);
    procedure AddHistoryItem(Item : String);

  public
    { Public declarations }
  end;

var
  frmMain: TfrmMain;

implementation
{$R *.DFM}


procedure MyLineOut(s: string); // Output procedure
begin
//  frmMain.memo1.lines.add(s);
  frmMain.WriteFromPseWow(s);
end;

procedure TfrmMain.FormCreate(Sender: TObject);
begin
  RC_LineOut:=MyLineOut; // set Output
  SetupIcons;
  LoadSettings;
  Ready := False;
end;

procedure TfrmMain.btnRunClick(Sender: TObject);
var
  IniFile : TInifile;
begin
  IniFile := TIniFile.Create(ExtractFilePath(Application.ExeName)+'Settings.INI');
  IniFile.WriteString('Execute','Application',txtExe.Text);
  IniFile.UpdateFile;
  IniFile.Free;
  RC_Run(txtExe.text); // run console program
end;

procedure TfrmMain.btnExitClick(Sender: TObject);
begin
  ShutDown();
end;

procedure TfrmMain.ShutDown;
begin
  Log('Shut down PseuWow Process',clRed);
  RC_LineIn('!');
  RC_LineIn('quit');
  Sleep(3000);
end;

procedure TfrmMain.Execute(AFile : String);
begin
  //TT: Get Info from PseuWow.conf
  LoadPseuSettings(ExtractFilePath(AFile)+'\conf\PseuWoW.conf');

  //TT: See if we already have a server running
  with clientSock do
  begin
    Port := 8089;
    Open;

    if (Active) then
    begin
      Close;
      servRemote.Active := False;
    end
    else
      servRemote.Active := True;
  end;


  Running := True;
  pnlTop.Hide;
  comCommand.SetFocus;
  RC_Run(AFile);

end;

procedure TfrmMain.FormCloseQuery(Sender: TObject; var CanClose: Boolean);
begin
  if RC_ExitCode = STILL_ACTIVE then
  begin
    ShutDown;
    CanClose := True;
  end;
end;

procedure TfrmMain.timerStartTimer(Sender: TObject);
begin
  timerStart.Enabled := False;
  if Ready then
  begin
    //TrayIcon.HideApplication;
    Launch;
    Exit;
  end
  else
  begin
    if clientSock.Active = false then
      clientSock.Active := True;
  end;

  timerStart.Enabled := True;
end;

procedure TfrmMain.servRemoteClientRead(Sender: TObject;
  Socket: TCustomWinSocket);
begin
  RC_LineIn('!');
  RC_LineIn(Socket.ReceiveText);
  Log('Received Remote Commad: ' + Socket.ReceiveText, clGreen );
end;

procedure TfrmMain.FormDestroy(Sender: TObject);
begin
  servRemote.Active := False;
end;

procedure TfrmMain.clientSockConnect(Sender: TObject;
  Socket: TCustomWinSocket);
begin

  //World Server Check
  if clientSock.Port = 8085 then
  begin
    Ready := True;
    clientSock.Active := False;
    Log('**** WS Is Ready For Connections ****');
    Launch;
  end;

  //Checking If We Have A listening Console
  if clientSock.Port = 8089 then
  begin
    Log('**** Already Listening ****');
  end;

end;

procedure TfrmMain.clientSockError(Sender: TObject;
  Socket: TCustomWinSocket; ErrorEvent: TErrorEvent;
  var ErrorCode: Integer);
begin
  //World Server Check
  if clientSock.Port = 8085 then
  begin
    Ready := False;
    clientSock.Active := False;
    Log('Still Waiting For Server',clMaroon);
    ErrorCode := 0;
  end
  else
  begin
    Log('Error in Checking For Listening', clMaroon);
    ErrorCode := 0;
  end;
end;

procedure TfrmMain.Launch;
var
  IniFile : TInifile;
  iIcon : Integer;
begin
  if Ready = False then
    Exit;

  Running := False;
  timerStart.Enabled := False;

  if App <> '' then
    Execute(App)
  else
  begin
    timerStart.Enabled := True;
  end;
end;


procedure TfrmMain.Log(AText: String; Color: TColor);
begin
  AddColouredLine(Console,'CONSOLE: '+AText, Color);
end;


procedure TfrmMain.ConsoleResizeRequest(Sender: TObject; Rect: TRect);
var
  ScrollMessage: TWMVScroll;
  i : Integer;
begin
  ScrollMessage.Msg := WM_VScroll;
  for i := 0 to Console.Lines.Count do
  begin
    ScrollMessage.ScrollCode := sb_LineDown;
    ScrollMessage.Pos := 0;
    Console.Dispatch(ScrollMessage);
  end;
end;

procedure TfrmMain.comCommandKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  if key = VK_RETURN then
  begin
    if ConsoleCommand(comCommand.Text) then
    begin
      key := 0;
      Exit;
    end;

    // send command line on Enter Key
    RC_LineIn(comCommand.Text);
    AddHistoryItem(comCommand.Text);
    comCommand.Text := '';
    key:=0;
  end;
end;

procedure TfrmMain.clientSockConnecting(Sender: TObject;
  Socket: TCustomWinSocket);
begin
  if clientSock.Port = 8085 then
  begin
    Log('Establishing Connection to WS',clGreen);
  end;

  if clientSock.Port = 8089 then
  begin
    Log('Checking For Listening Console',clGreen);
  end;
end;

procedure TfrmMain.WriteFromPseWow(AString: String);
begin
  AString := AnsiReplaceText(AString,'|r','');
  //This doesnt ADD any color at the moment it just seems to clean up the string a bit
  AddColourToLine(Console,AString);
end;

procedure TfrmMain.AddHistoryItem(Item: String);
begin
  with comCommand do
  begin
    if Items.IndexOf(Item) = -1 then
    begin
      Items.Add(Item);
    end;
  end;
end;

function TfrmMain.ConsoleCommand(AString: String): Boolean;
begin
  Result := False;
  AString := UpperCase(AString);

  if (AString = 'QUIT') or (AString = 'EXIT') then
  begin
    Result := True;
    ShutDown;
    Sleep(1000);
    Close;
  end;

end;

procedure TfrmMain.SetupIcons;
var
  i : Integer;
begin
  cbexIcon.Clear;
  
  for i := 0 to imgList.Count - 1 do
  begin
    cbexIcon.ItemsEx.AddItem('',i,i,i,0,nil);
  end;

end;

procedure TfrmMain.SetIcon(AIndex : Integer; AUpdateINI : Boolean = True);
var
  IniFile : TInifile;
begin
  try
    IniFile := TIniFile.Create(ExtractFilePath(Application.ExeName)+'Settings.INI');
    if AUpdateINI then
      IniFile.WriteInteger('Look','Icon',AIndex);

    with imgList do
    begin
      GetIcon(AIndex, Application.Icon);
      TrayIcon.IconIndex := AIndex;
    end;
    cbexIcon.ItemIndex := AIndex;
  finally
    if AUpdateINI then
    begin
      IniFile.UpdateFile;
      comCommand.SetFocus;
    end;
    IniFile.Free;
  end;
end;

procedure TfrmMain.LoadSettings;
var
  IniFile : TInifile;
  iIcon : Integer;
begin
  try
    //TT: Read from Inifile for the path the file we want.
    IniFile := TIniFile.Create(ExtractFilePath(Application.ExeName)+'Settings.INI');
    App := IniFile.ReadString('Execute','Application','');
    if App = '' then
    begin
      if FileExists(ExtractFilePath(Application.ExeName)+'pseuwow.exe') then
      begin
        App := ExtractFilePath(Application.ExeName)+'pseuwow.exe';
        pnlTop.Hide;
      end
      else
        pnlTop.Show;
    end;
    IniFile.WriteString('Execute','Application',App);

    //TT: Read Tray Icon, Nice for those of us who more than one session at a time!
    iIcon := IniFile.ReadInteger('Look','Icon',-1);

    if (iIcon = -1) then
      IniFile.WriteInteger('Look','Icon',0);

    SetIcon(iIcon, False);

  finally
    IniFile.UpdateFile;
    IniFile.Free;
  end;
end;

procedure TfrmMain.cbexIconChange(Sender: TObject);
begin
  if Ready then
    SetIcon(cbexIcon.ItemIndex);
end;

procedure TfrmMain.LoadPseuSettings(AConFile : string);
var
  fFile : textfile;
  sBuffer : string;
  sRes : string;
begin
  if FileExists(AConFile) then
  begin
    AssignFile(fFile, AConFile);
    Reset(fFile);

    while not(Eof(fFile)) do
    begin
      sRes := '';

      Readln(fFile, sBuffer);

      if EvaluateProperty(sBuffer, 'charname=', sRes) then
      begin
        txtChar.Caption := sRes;
        Application.Title := sRes + ' - PseuWoW Console';
        TrayIcon.Hint := Application.Title;
      end;
    end;
  end;
  CloseFile(fFile);

  comCommand.SetFocus;
end;

end.
