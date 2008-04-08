#ifndef SCENEDATA_H
#define SCENEDATA_H

#define SCENEDATA_SIZE 255

// I: index
// D: data value

enum SceneLoginDataIndexes
{
    ISCENE_LOGIN_CONN_STATUS,
    ISCENE_LOGIN_END
};

enum SceneLoginConnStatus
{
    DSCENE_LOGIN_NOT_CONNECTED,
    DSCENE_LOGIN_CONN_ATTEMPT,
    DSCENE_LOGIN_CONN_FAILED,
    DSCENE_LOGIN_LOGGING_IN,
    DSCENE_LOGIN_AUTHENTICATING,
    DSCENE_LOGIN_AUTH_FAILED,
    DSCENE_LOGIN_AUTH_OK
};
    

#endif
