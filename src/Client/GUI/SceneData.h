#ifndef SCENEDATA_H
#define SCENEDATA_H

#define SCENEDATA_SIZE 255

// I: index, enums should start with 1
// D: data value
// dummy indexes are used to keep space for misc text data in an SCP file

enum SceneLoginDataIndexes
{
    ISCENE_LOGIN_CONN_STATUS = 1,
    ISCENE_LOGIN_MSGBOX_DUMMY = 2,
    ISCENE_LOGIN_END = 3
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
    

#endif
