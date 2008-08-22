#include "common.h"
#include "PseuGUI.h"
#include "PseuWoW.h"
#include "Scene.h"
#include "RealmSession.h"

enum GuiElementID
{
    TEXTBOX_NAME = 0x1,
    TEXTBOX_PASSWORD = 0x2,
    BUTTON_QUIT = 0x4,
    BUTTON_COMMUNITY = 0x8,
    BUTTON_LOGON = 0x10,
};

class GUIEventReceiver : public IEventReceiver
{
public:
    GUIEventReceiver()
    {
        buttons=0;
    }
    virtual bool OnEvent(const SEvent& event)
    {
    //GUI EVENT
    if (event.EventType == EET_GUI_EVENT)
    {
        s32 id = event.GUIEvent.Caller->getID();

        switch(event.GUIEvent.EventType)
        {
            case EGET_BUTTON_CLICKED:
            logdebug("user clicked button %u",id);
            buttons+=id;
            return true;
            break;
        }
    }

    return false;
    }
    u32 buttons;
};



//TODO: Reposition elements on resize
//the code happens only ones, We need to, in the main loop ( usually while(driver->run()) ) set the
//positions of each gui element based on the current screensize ( gotten with driver->getScreenSize(); )

SceneLogin::SceneLogin(PseuGUI *gui) : Scene(gui)
{
    textdb = instance->dbmgr.GetDB("gui_login_text");
    msgbox_textid = 0;
    eventrecv = new GUIEventReceiver();
    device->setEventReceiver(eventrecv);
    root = guienv->getRootGUIElement();

    dimension2d<s32> scrn = driver->getScreenSize();

    irrlogo = guienv->addImage(driver->getTexture("data/misc/irrlichtlogo.png"), core::position2d<s32>(5,5),true,root);
    background = guienv->addImage(driver->getTexture("data/misc/sky.jpg"), core::position2d<s32>(5,5),true,root);
    background->setRelativePosition(rect<s32>(0,0,scrn.Width,scrn.Height));
    irrlogo->setScaleImage(true);

    guienv->addStaticText(L"Account:",rect<s32>((scrn.Width*0.5f)-90, (scrn.Height*0.3f)-10, (scrn.Width*0.5f)+90, (scrn.Height*0.3f)+10), false, false, 0, 0);
    guienv->addStaticText(L"Password:", rect<s32>((scrn.Width*0.5f)-90, (scrn.Height*0.3f)+50, (scrn.Width*0.5f)+90, (scrn.Height*0.3f)+70), false, false, 0, 0);
    guienv->addEditBox(L"", rect<s32>((scrn.Width*0.5f)-90, (scrn.Height*0.3f)+10, (scrn.Width*0.5f)+90, (scrn.Height*0.3f)+30), true, 0, 1);
    guienv->addEditBox(L"", rect<s32>((scrn.Width*0.5f)-90, (scrn.Height*0.3f)+70, (scrn.Width*0.5f)+90, (scrn.Height*0.3f)+90), true, 0, 2)->setPasswordBox(true);
    guienv->addButton(rect<s32>(scrn.Width-120, scrn.Height-40, scrn.Width-10, scrn.Height-10), 0, 4, L"Quit");
    guienv->addButton(rect<s32>(10, scrn.Height-40, 120, scrn.Height-10), 0, 8, L"Community Site");
    guienv->addButton(rect<s32>((scrn.Width*0.5f)-60, (scrn.Height*0.3f)+100, (scrn.Width*0.5f)+60, (scrn.Height*0.3f)+130), 0, 16, L"Logon");
    msgbox = guienv->addStaticText(GetStringFromDB(ISCENE_LOGIN_CONN_STATUS,DSCENE_LOGIN_NOT_CONNECTED).c_str(),rect<s32>((scrn.Width*0.5f)-90, (scrn.Height*0.3f)+150, (scrn.Width*0.5f)+90, (scrn.Height*0.3f)+180),true,true);

    if(soundengine)
    {
        soundengine->play2D("data/misc/main_theme.ogg",true);
    }
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
        eventrecv->buttons=0;
        logdebug("Commencing Logon");
        core::stringc tmp;
        tmp=root->getElementFromId(TEXTBOX_NAME,true)->getText();
        std::string accname =tmp.c_str();
        tmp=root->getElementFromId(TEXTBOX_PASSWORD,true)->getText();
        std::string accpass=tmp.c_str();
        if(accname.size() && accpass.size())
        {
            SetData(ISCENE_LOGIN_CONN_STATUS,DSCENE_LOGIN_CONN_ATTEMPT);
            logdebug("Trying to set Logon Data %u, %u", accname.size(), accpass.size());
            // we can safely override the conf settings
            instance->GetConf()->accname = accname;
            instance->GetConf()->accpass = accpass;
            instance->CreateRealmSession();
        }
        else
        {
            guienv->addMessageBox(GetStringFromDB(ISCENE_LOGIN_MSGBOX_DUMMY,0).c_str(),
                                  GetStringFromDB(ISCENE_LOGIN_MSGBOX_DUMMY,1).c_str());
        }

    }
    eventrecv->buttons = 0;
}

void SceneLogin::OnDelete(void)
{
    // sound will be stopped after char selection
    // not necessary to delete the images, because they are deleted by guienv->clear()
}

