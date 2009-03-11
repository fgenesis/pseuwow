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
DefReturnResult SCGetOpcodeName(CmdSet&);
DefReturnResult SCGetOpcodeID(CmdSet&);
DefReturnResult SCBBGetPackedGuid(CmdSet&);
DefReturnResult SCBBPutPackedGuid(CmdSet&);
DefReturnResult SCGui(CmdSet&);
DefReturnResult SCSendWho(CmdSet&);
DefReturnResult SCGetObjectDistance(CmdSet&);
DefReturnResult SCSwitchOpcodeHandler(CmdSet&);
DefReturnResult SCOpcodeDisabled(CmdSet&);
DefReturnResult SCSpoofWorldPacket(CmdSet&);
DefReturnResult SCLoadDB(CmdSet&);
DefReturnResult SCAddDBPath(CmdSet&);
DefReturnResult SCGetPos(CmdSet&);
DefReturnResult SCPreloadFile(CmdSet&);


void my_print(const char *fmt, ...);
void my_print_error(const char *fmt, ...);
void my_print_debug(const char *fmt, ...);

#ifdef PRINT
#undef PRINT
#define PRINT my_print
#endif

#ifdef PRINT_ERROR
#undef PRINT_ERROR
#define PRINT_ERROR my_print_error
#endif

#ifdef PRINT_DEBUG
#undef PRINT_DEBUG
#define PRINT_DEBUG my_print_debug
#endif

#endif
