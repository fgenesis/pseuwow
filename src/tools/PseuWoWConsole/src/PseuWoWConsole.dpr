program PseuWoWConsole;

uses
  Forms,
  fMain in 'fMain.pas' {frmMain},
  RedirectConsole in 'RedirectConsole.pas',
  modRichEdit in '..\..\..\Projects\Common\modRichEdit.pas';

{$R *.RES}

begin
  Application.Initialize;
  Application.Title := 'PseWoW Console';
  Application.CreateForm(TfrmMain, frmMain);
  Application.Run;
end.
