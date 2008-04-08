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

SceneLogin::SceneLogin(PseuGUI *gui) : Scene(gui)
{
    _gui=gui;
    eventrecv = new GUIEventReceiver();
    device->setEventReceiver(eventrecv);
    root = guienv->getRootGUIElement();

    dimension2d<s32> scrn = driver->getScreenSize();

    irrlogo = guienv->addImage(driver->getTexture("data/misc/irrlichtlogo.png"), core::position2d<s32>(5,5));
guienv->addStaticText(L"Password:",rect<s32>((scrn.Width*0.5f)-90, (scrn.Height*0.3f)-10, (scrn.Width*0.5f)+90, (scrn.Height*0.3f)-10), false, false, 0, 0);
guienv->addStaticText(L"Account:", rect<s32>((scrn.Width*0.5f)-90, (scrn.Height*0.3f)+50, (scrn.Width*0.5f)+90, (scrn.Height*0.3f)+70), false, false, 0, 0);
guienv->addEditBox(L"", rect<s32>((scrn.Width*0.5f)-90, (scrn.Height*0.3f)+10, (scrn.Width*0.5f)+90, (scrn.Height*0.3f)+30), true, 0, 1);
guienv->addEditBox(L"", rect<s32>((scrn.Width*0.5f)-90, (scrn.Height*0.3f)+70, (scrn.Width*0.5f)+90, (scrn.Height*0.3f)+90), true, 0, 2)->setPasswordBox(true);
guienv->addButton(rect<s32>(scrn.Width-120, scrn.Height-40, scrn.Width-10, scrn.Height-10), 0, 4, L"Quit");
guienv->addButton(rect<s32>(10, scrn.Height-40, 120, scrn.Height-10), 0, 8, L"Community Site");
guienv->addButton(rect<s32>((scrn.Width*0.5f)-60, (scrn.Height*0.3f)+100, (scrn.Width*0.5f)+60, (scrn.Height*0.3f)+130), 0, 16, L"Logon");


}

void SceneLogin::OnUpdate(s32 timepassed)
{
    if(eventrecv->buttons & BUTTON_QUIT)
    {
        _gui->Shutdown();
        _gui->GetInstance()->Stop();
        eventrecv->buttons=0;
    }
    if(eventrecv->buttons & BUTTON_LOGON)
    {
        logdebug("Commencing Logon");
        core::stringc tmp;
        tmp=root->getElementFromId(TEXTBOX_NAME,true)->getText();
        std::string accname =tmp.c_str();
        tmp=root->getElementFromId(TEXTBOX_PASSWORD,true)->getText();
        std::string accpass=tmp.c_str();
        logdebug("Trying to set Logon Data %u, %u", accname.size(), accpass.size());
        _gui->GetInstance()->GetRSession()->SetLogonData(accname,accpass);
        _gui->GetInstance()->GetRSession()->SendLogonChallenge();
        eventrecv->buttons=0;
        _gui->GetInstance()->login=true;
    }
}

void SceneLogin::OnDelete(void)
{
    // not necessary to delete the images, because they are deleted by guienv->clear()
}

