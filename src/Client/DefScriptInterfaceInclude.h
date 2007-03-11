#ifndef DEFSCRIPTINTERFACEINCLUDE_H
#define DEFSCRIPTINTERFACEINCLUDE_H
#define USING_DEFSCRIPT_EXTENSIONS true

void _InitDefScriptInterface();
// Useable own internal functions:
DefReturnResult SCpause(CmdSet&);
DefReturnResult SCSendChatMessage(CmdSet&);
DefReturnResult SCsavecache(CmdSet&);
DefReturnResult SCemote(CmdSet&);
DefReturnResult SCfollow(CmdSet&);
DefReturnResult SCshdn(CmdSet&);
DefReturnResult SCjoinchannel(CmdSet&);
DefReturnResult SCleavechannel(CmdSet&);
DefReturnResult SCloadconf(CmdSet&);
DefReturnResult SCapplypermissions(CmdSet&);
DefReturnResult SCapplyconf(CmdSet&);
DefReturnResult SClog(CmdSet&);
DefReturnResult SClogdetail(CmdSet&);
DefReturnResult SClogdebug(CmdSet&);
DefReturnResult SClogerror(CmdSet&);
DefReturnResult SCcastspell(CmdSet&);
DefReturnResult SCqueryitem(CmdSet&);
DefReturnResult SCtarget(CmdSet&);
DefReturnResult SCloadscp(CmdSet&);
DefReturnResult SCScpExists(CmdSet&);
DefReturnResult SCScpSectionExists(CmdSet&);
DefReturnResult SCScpEntryExists(CmdSet&);
DefReturnResult SCGetScpValue(CmdSet&);

#endif