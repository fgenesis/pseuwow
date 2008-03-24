/////////////////////////////////////////////////////
//                                                 //
//   UNiT REDiRECT CONSOLE by SONiC                //
//                                                 //
//   Console input/output redirection with pipes   //
//   Last revision: 02/SEPT/02                     //
//                                                 //
//   Bugs/comments to: Sonic1980@msn.com           //
//   Home page: http://sonic.rulestheweb.com       //
//                                                 //
//   Freeware                                      //
//                                                 //
/////////////////////////////////////////////////////

unit RedirectConsole;

interface

const
  CRLF=#13#10;

var
  RC_SendBuf: string;
  RC_End: Boolean;
  RC_ExitCode: Cardinal;

procedure RC_Run(Command: string);
procedure RC_LineIn(s: string);
var       RC_LineOut: procedure(s: string);

implementation

uses Windows, Forms;

procedure RC_LineIn(s: string);
begin
  RC_SendBuf:=RC_SendBuf+s+CRLF;
end; // RC_LineIn;

function IsWinNT: Boolean;
var osv: tOSVERSIONINFO;
begin
  osv.dwOSVersionInfoSize:=sizeof(osv);
  GetVersionEx(osv);
  result:=osv.dwPlatformID=VER_PLATFORM_WIN32_NT;
end; // IsWinNT

procedure SplitLines(s: string);
var t: string;
begin
  while pos(CRLF, s)<>0 do begin
    t:=copy(s, 1, pos(CRLF, s)-1);
    RC_LineOut(t);
    delete(s, 1, pos(CRLF, s)+1);
  end;
  if length(s)>0 then RC_LineOut(s);
end; // SplitLines

procedure RC_Run(Command: string);
const bufsize=1024; // 1KByte buffer
var
  buf: array [0..bufsize-1] of char;
  si: tSTARTUPINFO;
  sa: tSECURITYATTRIBUTES;
  sd: tSECURITYDESCRIPTOR;
  pi: tPROCESSINFORMATION;
  newstdin, newstdout, read_stdout, write_stdin: tHandle;
  bread, avail: dword;
begin
  // Configuraciones de seguridad para WinNT
  if IsWinNT then begin
    InitializeSecurityDescriptor(@sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(@sd, true, nil, false);
    sa.lpSecurityDescriptor:=@sd;
  end else sa.lpSecurityDescriptor:=nil;
  // Creamos Pipe A
  if not CreatePipe(newstdin, write_stdin, @sa, 0) then begin
    RC_LineOut('Error creating Pipe A');
    exit;
  end;
  // Creamos Pipe B
  if not CreatePipe(read_stdout, newstdout, @sa, 0) then begin
    RC_LineOut('Error creating Pipe B');
    CloseHandle(newstdin);
    CloseHandle(write_stdin);
    exit;
  end;
  // Configuramos si
  GetStartupInfo(si);
  si.dwFlags:=STARTF_USESTDHANDLES or STARTF_USESHOWWINDOW;
  si.wShowWindow:=SW_HIDE;
  si.hStdOutput:=newstdout;
  si.hStdError:=newstdout;
  si.hStdInput:=newstdin;
  // Creamos proceso
  if not CreateProcess(pchar(command), nil, nil, nil, true,
    CREATE_NEW_CONSOLE, nil, nil, si, pi) then begin
    RC_LineOut('Error creating process: '+command);
    CloseHandle(newstdin);
    CloseHandle(newstdout);
    CloseHandle(read_stdout);
    CloseHandle(write_stdin);
    exit;
  end;
  // Loop principal
  fillchar(buf, sizeof(buf), 0);
  RC_End:=false;
  RC_SendBuf:='';
  repeat
    // application.processmessages;
    Application.HandleMessage;
    GetExitCodeProcess(pi.hProcess, RC_ExitCode);
    if (RC_ExitCode<>STILL_ACTIVE) then RC_End:=True;
    PeekNamedPipe(read_stdout, @buf, bufsize, @bread, @avail, nil);
    // Comprobamos texto de salida
    if (bread<>0) then begin
      fillchar(buf, bufsize, 0);
      if (avail>bufsize) then
        while (bread>=bufsize) do begin
          ReadFile(read_stdout, buf, bufsize, bread, nil);
          SplitLines(buf);
          fillchar(buf, bufsize, 0);
        end
      else begin
        ReadFile(read_stdout, buf, bufsize, bread, nil);
        SplitLines(buf);
      end;
    end;
    // Comprobamos texto de entrada
    while (Length(RC_SendBuf)>0) do begin
      WriteFile(write_stdin, RC_SendBuf[1], 1, bread, nil);
      Delete(RC_SendBuf, 1, 1);
    end;
  until RC_End;
  // Cerramos las cosas
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);
  CloseHandle(newstdin);
  CloseHandle(newstdout);
  CloseHandle(read_stdout);
  CloseHandle(write_stdin);
end; // RC_Run

end.
