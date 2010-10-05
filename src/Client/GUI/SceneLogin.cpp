#include "common.h"
#include "PseuGUI.h"
#include "PseuWoW.h"
#include "Scene.h"
#include "RealmSession.h"
#include "GUIEventReceiver.h"

enum GuiElementID
{
    TEXTBOX_NAME = 0x1,
    TEXTBOX_PASSWORD = 0x2,
    BUTTON_QUIT = 0x4,
    BUTTON_COMMUNITY = 0x8,
    BUTTON_LOGON = 0x10,
};

//TODO: Reposition elements on resize
//the code happens only ones, We need to, in the main loop ( usually while(driver->run()) ) set the
//positions of each gui element based on the current screensize ( gotten with driver->getScreenSize(); )

SceneLogin::SceneLogin(PseuGUI *gui) : Scene(gui)
{
    textdb = instance->dbmgr.GetDB("gui_login_text");
    msgbox_textid = 0;
    eventrecv = new GUIEventReceiver();
    eventrecv->keyToButtonMap[KEY_RETURN] = BUTTON_LOGON;
    eventrecv->keyToButtonMap[KEY_ESCAPE] = BUTTON_QUIT;
    device->setEventReceiver(eventrecv);

    dimension2d<s32> scrn = driver->getScreenSize();

    irrlogo = guienv->addImage(driver->getTexture("data/misc/irrlichtlogo.png"), core::position2d<s32>(5,5),true,rootgui);
    background = guienv->addImage(driver->getTexture("data/misc/sky.jpg"), core::position2d<s32>(5,5),true,rootgui);
    background->setRelativePosition(rect<s32>(0,0,scrn.Width,scrn.Height));
    irrlogo->setScaleImage(true);

    core::stringw accn;
    accn += instance->GetConf()->accname.c_str();

    guienv->addStaticText(GetStringFromDB(ISCENE_LOGIN_LABELS,DSCENE_LOGIN_LABEL_ACC).c_str(),rect<s32>((scrn.Width*0.5f)-90, (scrn.Height*0.3f)-10, (scrn.Width*0.5f)+90, (scrn.Height*0.3f)+10), false, false, 0, 0);
    guienv->addStaticText(GetStringFromDB(ISCENE_LOGIN_LABELS,DSCENE_LOGIN_LABEL_PASS).c_str(), rect<s32>((scrn.Width*0.5f)-90, (scrn.Height*0.3f)+50, (scrn.Width*0.5f)+90, (scrn.Height*0.3f)+70), false, false, 0, 0);
    guienv->addEditBox(accn.c_str(), rect<s32>((scrn.Width*0.5f)-90, (scrn.Height*0.3f)+10, (scrn.Width*0.5f)+90, (scrn.Height*0.3f)+30), true, 0, 1);
    guienv->addEditBox(L"", rect<s32>((scrn.Width*0.5f)-90, (scrn.Height*0.3f)+70, (scrn.Width*0.5f)+90, (scrn.Height*0.3f)+90), true, 0, 2)->setPasswordBox(true);
    guienv->addButton(rect<s32>(scrn.Width-120, scrn.Height-40, scrn.Width-10, scrn.Height-10), 0, 4, GetStringFromDB(ISCENE_LOGIN_BUTTONS,DSCENE_LOGIN_BUTTON_QUIT).c_str());
    guienv->addButton(rect<s32>(10, scrn.Height-40, 120, scrn.Height-10), 0, 8, GetStringFromDB(ISCENE_LOGIN_BUTTONS,DSCENE_LOGIN_BUTTON_SITE).c_str());
    guienv->addButton(rect<s32>((scrn.Width*0.5f)-60, (scrn.Height*0.3f)+100, (scrn.Width*0.5f)+60, (scrn.Height*0.3f)+130), 0, 16, GetStringFromDB(ISCENE_LOGIN_BUTTONS,DSCENE_LOGIN_BUTTON_LOGIN).c_str());
    msgbox = guienv->addStaticText(GetStringFromDB(ISCENE_LOGIN_CONN_STATUS,DSCENE_LOGIN_NOT_CONNECTED).c_str(),rect<s32>((scrn.Width*0.5f)-90, (scrn.Height*0.3f)+150, (scrn.Width*0.5f)+90, (scrn.Height*0.3f)+180),true,true);

    if(soundengine)
    {
        ISoundSource *main_theme = soundengine->getSoundSource("data/misc/main_theme.ogg");
        if(main_theme && !soundengine->isCurrentlyPlaying(main_theme))
        {
            soundengine->play2D(main_theme,true);
        }
    }

    popup = NULL;
}

void SceneLogin::OnUpdate(s32 timepassed)
{
    if(msgbox_textid != scenedata[ISCENE_LOGIN_CONN_STATUS])
    {
        msgbox_textid = scenedata[ISCENE_LOGIN_CONN_STATUS];
        msgbox->setText(GetStringFromDB(ISCENE_LOGIN_CONN_STATUS,msgbox_textid).c_str());
    }

    if(eventrecv->buttons & BUTTON_QUIT)
    {
        instance->Stop();
    }
    if(eventrecv->buttons & BUTTON_LOGON)
    {
        logdebug("Commencing Logon");
        core::stringc tmp;
        tmp=rootgui->getElementFromId(TEXTBOX_NAME,true)->getText();
        std::string accname =tmp.c_str();
        tmp=rootgui->getElementFromId(TEXTBOX_PASSWORD,true)->getText();
        std::string accpass=tmp.c_str();
        if(accname.size() && accpass.size())
        {
            SetData(ISCENE_LOGIN_CONN_STATUS,DSCENE_LOGIN_CONN_ATTEMPT);
            logdebug("Trying to set Logon Data %u, %u", accname.size(), accpass.size());
            // we can safely override the conf settings
            instance->GetConf()->accname = accname;
            instance->GetConf()->accpass = accpass;
            // ...but do not set the defscript vars; its just not safe
            instance->CreateRealmSession();
        }
        else
        {
            popup = guienv->addMessageBox(GetStringFromDB(ISCENE_LOGIN_MSGBOX_DUMMY,0).c_str(),
                                        GetStringFromDB(ISCENE_LOGIN_MSGBOX_DUMMY,1).c_str());
            eventrecv->react_to_keys = false; // prevent main window from processing key input; it must be processed by the msgbox's event recv!
                                              // our eventrecv will handle msgbox close event by itself and enable input again.
        }

    }
    if(eventrecv->buttons & BUTTON_COMMUNITY)
    {
#if PLATFORM == PLATFORM_WIN32
        ShellExecute(NULL, "open", "http://www.mangosclient.org", NULL, NULL, SW_SHOWNORMAL);
#elif PLATFORM == PLATFORM_UNIX
        // linux code here
#elif PLATFORM == PLATFORM_APPLE
        // mac code here
#endif
    }
    eventrecv->buttons = 0;
}

void SceneLogin::OnDelete(void)
{
    // sound will be stopped after char selection
    // not necessary to delete the images, because they are deleted by guienv->clear()
}

