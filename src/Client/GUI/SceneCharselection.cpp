#include "common.h"
#include "PseuGUI.h"
#include "PseuWoW.h"
#include "Scene.h"
#include "GUIEventReceiver.h"
#include "RealmSession.h"
#include "WorldSession.h"

enum GuiElementID
{
    BUTTON_ENTER_WORLD = 0x1,
    BUTTON_BACK = 0x2,
    BUTTON_NEW_CHARACTER = 0x4,
    BUTTON_DELETE_CHARACTER = 0x8,
    BUTTON_SELECT_REALM = 0x10,
    BUTTON_REALMWIN_OK = 0x20,
    BUTTON_REALMWIN_CANCEL = 0x40,
};


SceneCharSelection::SceneCharSelection(PseuGUI *gui) : Scene(gui)
{
    textdb = instance->dbmgr.GetDB("gui_charselect_text");
    eventrecv = new GUIEventReceiver();
    device->setEventReceiver(eventrecv);
    eventrecv->keyToButtonMap[KEY_ESCAPE] = BUTTON_BACK | BUTTON_REALMWIN_CANCEL;
    eventrecv->keyToButtonMap[KEY_RETURN] = BUTTON_ENTER_WORLD | BUTTON_REALMWIN_OK;
    eventrecv->customHandledEvents.insert(EGET_LISTBOX_SELECTED_AGAIN);

    dimension2d<s32> scrn = driver->getScreenSize();

    OnResize(); // call this manually to draw all buttons and stuff

    if(soundengine)
    {
        ISoundSource *main_theme = soundengine->getSoundSource("data/misc/main_theme.ogg");
        if(main_theme && !soundengine->isCurrentlyPlaying(main_theme))
        {
            soundengine->play2D(main_theme,true);
        }
    }
}

void SceneCharSelection::OnUpdate(s32 timepassed)
{
    // treat doubleclick on listboxes as OK button click
    if(!eventrecv->guievent_proc)
    {
        eventrecv->guievent_proc = true;
        if(eventrecv->guievent.GUIEvent.EventType == EGET_LISTBOX_SELECTED_AGAIN)
        {
            if(eventrecv->guievent.GUIEvent.Caller == realmlistbox)
            {
                eventrecv->buttons |= BUTTON_REALMWIN_OK;
            }
            else if(eventrecv->guievent.GUIEvent.Caller == charlistbox)
            {
                eventrecv->buttons |= BUTTON_ENTER_WORLD;
            }
        }
    }

    if(eventrecv->buttons & BUTTON_ENTER_WORLD && !realmwin)
    {
        logdebug("GUI: SceneCharSelect: Entering world");
        WorldSession *ws = instance->GetWSession();
        if(ws)
        {
            u32 selected = charlistbox->getSelected();
            if(selected < ws->GetCharsCount())
            {
                ws->EnterWorldWithCharacter(ws->GetCharFromList(selected).p._name);
            }
            else
                logerror("Character selection out of bounds! (%u)",selected);
        }
        else
            logerror("GUI: BUTTON_ENTER_ WORLD pressed, but no WorldSession exists!");
    }
    if(eventrecv->buttons & BUTTON_BACK && !realmwin) // cant cancel with realmwin open (important for ESC key handling)
    {
        logdebug("GUI: SceneCharSelect: Back to Loginscreen");
        gui->SetSceneState(SCENESTATE_LOGINSCREEN);
        // disconnect from realm server if connected
        if(RealmSession *rs = instance->GetRSession())
            rs->SetMustDie();
        if(WorldSession *ws = instance->GetWSession())
            ws->SetMustDie();
    }
    if(eventrecv->buttons & BUTTON_DELETE_CHARACTER)
    {
        guienv->addMessageBox(L"Not yet implemented!", L"Deleting a character does not yet work!");
    }
    if(eventrecv->buttons & BUTTON_NEW_CHARACTER)
    {
        guienv->addMessageBox(L"Not yet implemented!", L"Creating a new character does not yet work!");
    }
    if(eventrecv->buttons & BUTTON_SELECT_REALM)
    {
        if(instance->GetRSession())
        {
            scenedata[ISCENE_CHARSEL_REALMFIRST] = 1; // show realm selection window
            OnResize(); // force gui redraw to remove window
            // TODO: there should exist a better way without full redraw
        }
        else
        {
            guienv->addMessageBox(L"Not yet implemented!", L"This action is not yet supported.\nYou can change the realm only while still connected to the realm server.");
        }
    }    
    if(eventrecv->buttons & BUTTON_REALMWIN_OK)
    {
        RealmSession *rs = instance->GetRSession();
        if(rs)
        {
            u32 selected = realmlistbox->getSelected();
            if(selected < rs->GetRealmCount())
            {
                rs->SetRealmAddr(rs->GetRealm(selected).addr_port);
                instance->CreateWorldSession();
                eventrecv->buttons |= BUTTON_REALMWIN_CANCEL; // easiest way to close the window without much additional code
            }
            else
                logerror("Realmlist selection out of bounds! (%u)",selected);
        }
    }


    // realmlist window
    if(eventrecv->buttons & BUTTON_REALMWIN_CANCEL)
    {
        scenedata[ISCENE_CHARSEL_REALMFIRST] = 0; // do not show realm selection window
        OnResize(); // force gui redraw to remove window
        // TODO: there should exist a better way without full redraw
    }

    eventrecv->buttons = 0;
}

void SceneCharSelection::OnDelete(void)
{
}


void SceneCharSelection::OnResize(void)
{
    // this seems to be necessary since after a window resize <textdb> == 0x00000001 for some unknown reason (=CRASH!)
    // if anyone knows how to fix that or whats causing it... please do so.
    // note: used VC71. also tested it with resizing SceneLogin, there the ptr stays fine. wtf?!
    textdb = instance->dbmgr.GetDB("gui_charselect_text");

    guienv->clear(); // drop all elements and redraw
    guienv->addButton(CalcRelativeScreenPos(driver, 0.45f ,0.9f, 0.15f, 0.05f), NULL, BUTTON_ENTER_WORLD,
        GetStringFromDB(ISCENE_CHARSEL_BUTTONS, DSCENE_CHARSEL_BUTTON_ENTERWORLD).c_str());
    guienv->addButton(CalcRelativeScreenPos(driver, 0.9f, 0.92f, 0.08f, 0.03f), NULL, BUTTON_BACK,
        GetStringFromDB(ISCENE_CHARSEL_BUTTONS, DSCENE_CHARSEL_BUTTON_BACK).c_str());
    guienv->addButton(CalcRelativeScreenPos(driver, 0.85f, 0.05f, 0.12f, 0.04f), NULL, BUTTON_SELECT_REALM,
        GetStringFromDB(ISCENE_CHARSEL_BUTTONS, DSCENE_CHARSEL_BUTTON_CHANGEREALM).c_str());
    guienv->addButton(CalcRelativeScreenPos(driver, 0.85f, 0.8f, 0.1f, 0.04f), NULL, BUTTON_NEW_CHARACTER,
        GetStringFromDB(ISCENE_CHARSEL_BUTTONS, DSCENE_CHARSEL_BUTTON_NEWCHAR).c_str());
    guienv->addButton(CalcRelativeScreenPos(driver, 0.78f, 0.92f, 0.1f, 0.03f), NULL, BUTTON_DELETE_CHARACTER,
        GetStringFromDB(ISCENE_CHARSEL_BUTTONS, DSCENE_CHARSEL_BUTTON_DELCHAR).c_str());

    realmwin = NULL;
    mutex.acquire();
    if(scenedata[ISCENE_CHARSEL_REALMFIRST])
    {
        dimension2d<s32> dim;
        rect<s32> pos;
        realmwin = guienv->addWindow(CalcRelativeScreenPos(driver, 0.2f, 0.2f, 0.6f, 0.6f), true,
            GetStringFromDB(ISCENE_CHARSEL_LABELS, DSCENE_CHARSEL_LABEL_REALMWIN).c_str());
        pos = realmwin->getAbsolutePosition(); // get absolute position and trandform <dim> to absolute in-window position
        dim.Width = pos.LowerRightCorner.X - pos.UpperLeftCorner.X;
        dim.Height = pos.LowerRightCorner.Y - pos.UpperLeftCorner.Y;
        realmwin->addChild(guienv->addButton(CalcRelativeScreenPos(dim, 0.7f, 0.93f, 0.12f, 0.05f), realmwin, BUTTON_REALMWIN_OK,
            GetStringFromDB(ISCENE_CHARSEL_BUTTONS, DSCENE_CHARSEL_REALMWIN_OK).c_str()));
        realmwin->addChild(guienv->addButton(CalcRelativeScreenPos(dim, 0.85f, 0.93f, 0.12f, 0.05f), realmwin, BUTTON_REALMWIN_CANCEL,
            GetStringFromDB(ISCENE_CHARSEL_BUTTONS, DSCENE_CHARSEL_REALMWIN_CANCEL).c_str()));

        realmlistbox = guienv->addListBox(CalcRelativeScreenPos(dim, 0.1f, 0.1f, 0.8f, 0.8f), realmwin);
        realmwin->addChild(realmlistbox);

        RealmSession *rs = instance->GetRSession();

        for(uint32 i = 0; i < rs->GetRealmCount(); i++)
        {
            SRealmInfo& r = rs->GetRealm(i);
            core::stringw entry;
            entry += r.name.c_str();
            entry += L", ";
            switch(r.icon) // icon means here RealmType
            {
                case 0: entry += "Normal"; break;
                case 1: entry += "PvP"; break;
                case 4: entry += "Normal(4)"; break;
                case 6: entry += "RP"; break;
                case 8: entry += "RP-PvP"; break;
                case 16: entry += "FFA-PvP"; break; // MaNGOS custom realm type
                default: entry += "Unknown"; break;
            }
            entry += L", (";
            entry += r.chars_here;
            entry += L") Chars";
            entry += L"      [";
            entry += r.addr_port.c_str();
            entry += L"]";
            realmlistbox->addItem(entry.c_str(), -1);
            SColor col;
            switch(r.color)
            {
                case 0: col.set(0xFF, 0x00, 0xFF, 0x00); break;
                case 1: col.set(0xFF, 0xFF, 0x00, 0x00); break;
                case 2: col.set(0xFF, 0x7F, 0x7F, 0x7F); break;
                case 3: col.set(0xFF, 0xB0, 0xB0, 0x00); break;
                default: col.set(0xFFFFFFFF);
            }
            realmlistbox->setItemOverrideColor(i,EGUI_LBC_TEXT,col);
            realmlistbox->setItemOverrideColor(i,EGUI_LBC_TEXT_HIGHLIGHT,col);
        }
        if(realmlistbox->getItemCount())
            realmlistbox->setSelected(0);
    }

    rect<s32> clb_rect = CalcRelativeScreenPos(driver, 0.65f, 0.12f, 0.34f, 0.67f);
    charlistbox = guienv->addListBox(clb_rect);
    WorldSession *ws = instance->GetWSession();
    if(ws)
    {
        SCPDatabase *racedb = instance->dbmgr.GetDB("race");
        uint32 ffaction = racedb->GetFieldId("faction");
        for(uint32 i = 0; i < ws->GetCharsCount(); i++)
        {
            CharacterListExt& c = ws->GetCharFromList(i);
            core::stringw entry;
            entry += c.p._name.c_str();
            entry += L", ";
            entry += L"Level ";
            entry += c.p._level;
            entry += "L ";
            entry += c.race.c_str();
            entry += L" ";
            entry += c.class_.c_str();
            entry += L", ";
            entry += c.zone.c_str();
            entry += L" (";
            entry += c.map_.c_str();
            entry += L")";
            charlistbox->addItem(entry.c_str());

            uint32 faction = racedb->GetInt(c.p._race, ffaction);
            
            SColor col;
            switch(faction)
            {
                case 1: col.set(0xFF, 0xFF, 0x30, 0x30); break;
                case 7: col.set(0xFF, 0x30, 0x30, 0xFF); break;
                default: col.set(0xFFFFFFFF);
            }
            charlistbox->setItemOverrideColor(i,EGUI_LBC_TEXT,col);
            charlistbox->setItemOverrideColor(i,EGUI_LBC_TEXT_HIGHLIGHT,col);

        }
    }
    mutex.release();

}

