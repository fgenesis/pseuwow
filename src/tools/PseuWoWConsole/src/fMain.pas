unit fMain;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, RedirectConsole, ExtCtrls, IniFiles, ScktComp, JvComponentBase,
  JvTrayIcon, ComCtrls, modRichEdit, StrUtils;

type
  TfrmMain = class(TForm)
    Panel1: TPanel;
    txtExe: TEdit;
    btnRun: TButton;
    btnExit: TButton;
    servRemote: TServerSocket;
    timerStart: TTimer;
    clientSock: TClientSocket;
    TrayIcon: TJvTrayIcon;
    Console: TRichEdit;
    grpCmd: TGroupBox;
    comCommand: TComboBox;
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
  private
    { Private declarations }
    App : String;
    Running : Boolean;
    Ready : Boolean;

    function ConsoleCommand(AString : String):Boolean;

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
  servRemote.Active := True;
  Running := True;
  Panel1.Hide;
  RC_Run(AFile);
  comCommand.SetFocus;
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
    TrayIcon.HideApplication;
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
  Ready := True;
  clientSock.Active := False;
  Log('**** WS Is Ready For Connections ****');
  Launch;
end;

procedure TfrmMain.clientSockError(Sender: TObject;
  Socket: TCustomWinSocket; ErrorEvent: TErrorEvent;
  var ErrorCode: Integer);
begin
  Ready := False;
  clientSock.Active := False;
  Log('Still Waiting For Server',clMaroon);
  ErrorCode := 0;
end;

procedure TfrmMain.Launch;
var
  IniFile : TInifile;
begin
  if Ready = False then
    Exit;

  Running := False;
  timerStart.Enabled := False;

  //TT: Read from Inifile for the path the file we want.
  IniFile := TIniFile.Create(ExtractFilePath(Application.ExeName)+'Settings.INI');
  App := IniFile.ReadString('Execute','Application','');
  if App = '' then
  begin
    if FileExists(ExtractFilePath(Application.ExeName)+'pseuwow.exe') then
      App := ExtractFilePath(Application.ExeName)+'pseuwow.exe';
  end;
  IniFile.WriteString('Execute','Application',App);
  IniFile.UpdateFile;
  IniFile.Free;

  if App <> '' then
    Execute(App);
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
  Log('Establishing Connection to WS',clGreen);
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

end.
