#ifndef DEFSCRIPTINTERFACEINCLUDE_H
#define DEFSCRIPTINTERFACEINCLUDE_H
#define USING_DEFSCRIPT_EXTENSIONS true

void _InitDefScriptInterface(void);
// Usable own internal functions:
DefReturnResult SCpause(CmdSet&);
DefReturnResult SCSendChatMessage(CmdSet&);
DefReturnResult SCsavecache(CmdSet&);
DefReturnResult SCemote(CmdSet&);
DefReturnResult SCfollow(CmdSet&);
DefReturnResult SCshdn(CmdSet&);
DefReturnResult SCjoinchannel(CmdSet&);
DefReturnResult SCleavechannel(CmdSet&);
DefReturnResult SClistchannel(CmdSet&);
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
DefReturnResult SCGetName(CmdSet&);
DefReturnResult SCGetPlayerGuid(CmdSet&);
DefReturnResult SCGetEntry(CmdSet&);
DefReturnResult SCGetItemProtoValue(CmdSet&);
DefReturnResult SCGetObjectType(CmdSet&);
DefReturnResult SCObjectKnown(CmdSet&);
DefReturnResult SCGetPlayerPerm(CmdSet&);
DefReturnResult SCGetScriptPerm(CmdSet&);
DefReturnResult SCGetFileList(CmdSet&);
DefReturnResult SCPrintScript(CmdSet&);
DefReturnResult SCGetObjectValue(CmdSet&);
DefReturnResult SCGetRace(CmdSet&);
DefReturnResult SCGetClass(CmdSet&);
DefReturnResult SCSendWorldPacket(CmdSet&);

#endif