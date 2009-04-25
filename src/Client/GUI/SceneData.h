#ifndef SCENEDATA_H
#define SCENEDATA_H

#define SCENEDATA_SIZE 256

// I: index, enums should start with 1
// D: data value
// dummy indexes are used to keep space for misc text data in an SCP file

enum SceneLoginDataIndexes
{
    ISCENE_LOGIN_CONN_STATUS = 1,
    ISCENE_LOGIN_MSGBOX_DUMMY = 2, // text
    ISCENE_LOGIN_LABELS = 3, // text
    ISCENE_LOGIN_BUTTONS = 4, // text
    ISCENE_LOGIN_END
};

enum SceneLoginConnStatus
{
    DSCENE_LOGIN_NOT_CONNECTED = 0,
    DSCENE_LOGIN_CONN_ATTEMPT = 1,
    DSCENE_LOGIN_CONN_FAILED = 2,
    DSCENE_LOGIN_ACC_NOT_FOUND = 3,
    DSCENE_LOGIN_ALREADY_CONNECTED = 4,
    DSCENE_LOGIN_WRONG_VERSION = 5,
    DSCENE_LOGIN_LOGGING_IN = 6,
    DSCENE_LOGIN_AUTHENTICATING = 7,
    DSCENE_LOGIN_AUTH_FAILED = 8,
    DSCENE_LOGIN_REQ_REALM = 9,
    DSCENE_LOGIN_UNK_ERROR = 10,
    DSCENE_LOGIN_FILE_TRANSFER = 11,
};

enum SceneLoginLabels
{
    DSCENE_LOGIN_LABEL_ACC = 0,
    DSCENE_LOGIN_LABEL_PASS = 1,
};

enum SceneLoginButtons
{
    DSCENE_LOGIN_BUTTON_LOGIN = 0,
    DSCENE_LOGIN_BUTTON_QUIT = 1,
    DSCENE_LOGIN_BUTTON_SITE = 2,
};

enum SceneCharSelectDataIndexes
{
    ISCENE_CHARSEL_BUTTONS = 1, // text
    ISCENE_CHARSEL_LABELS = 2, // text
    ISCENE_CHARSEL_ERRMSG = 3, // uint32 response code, see enum ResponseCodes in SharedDefines.h for IDs
    ISCENE_CHARSEL_REALMFIRST = 255, // flag that is set when connecting to a realm wasnt possible and the realm list must be shown first
    ISCENE_CHARSEL_END
};

enum SceneCharSelectButtons
{
    DSCENE_CHARSEL_BUTTON_ENTERWORLD = 0,
    DSCENE_CHARSEL_BUTTON_NEWCHAR = 1,
    DSCENE_CHARSEL_BUTTON_DELCHAR = 2,
    DSCENE_CHARSEL_BUTTON_CHANGEREALM = 3,
    DSCENE_CHARSEL_BUTTON_BACK = 4,
    DSCENE_CHARSEL_REALMWIN_OK = 5,
    DSCENE_CHARSEL_REALMWIN_CANCEL = 6,
    DSCENE_CHARSEL_NEWCHARWIN_OK = 7,
    DSCENE_CHARSEL_NEWCHARWIN_CANCEL = 8,
};

enum SceneCharSelectLabels
{
    DSCENE_CHARSEL_LABEL_REALMWIN = 0,
    DSCENE_CHARSEL_LABEL_NEWCHARWIN = 1,
};


    

#endif
