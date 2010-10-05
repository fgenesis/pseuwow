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
    BUTTON_NEWCHARWIN_OK = 0x80,
    BUTTON_NEWCHARWIN_CANCEL = 0x100,
};


SceneCharSelection::SceneCharSelection(PseuGUI *gui) : Scene(gui)
{
    realmwin = NULL;
    newcharwin = NULL;
    
    textdb = instance->dbmgr.GetDB("gui_charselect_text");
    racedb = instance->dbmgr.GetDB("race");
    classdb = instance->dbmgr.GetDB("class");
    eventrecv = new GUIEventReceiver();
    device->setEventReceiver(eventrecv);
    eventrecv->keyToButtonMap[KEY_ESCAPE] = BUTTON_BACK | BUTTON_REALMWIN_CANCEL | BUTTON_NEWCHARWIN_CANCEL;
    eventrecv->keyToButtonMap[KEY_RETURN] = BUTTON_ENTER_WORLD | BUTTON_REALMWIN_OK | BUTTON_NEWCHARWIN_OK;
    eventrecv->customHandledEvents.insert(EGET_LISTBOX_SELECTED_AGAIN);
    eventrecv->store_mouse = false; // do not queue mouse input

    dimension2d<s32> scrn = driver->getScreenSize();
    
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
    rect<s32> clb_rect = CalcRelativeScreenPos(driver, 0.65f, 0.12f, 0.34f, 0.67f);
    charlistbox = guienv->addListBox(clb_rect);

    FillCharlist();

    if(soundengine)
    {
        ISoundSource *main_theme = soundengine->getSoundSource("data/misc/main_theme.ogg");
        if(main_theme && !soundengine->isCurrentlyPlaying(main_theme))
        {
            soundengine->play2D(main_theme,true);
        }
    }
}

void SceneCharSelection::FillCharlist(void)
{
    ZThread::Guard<ZThread::FastMutex> g(mutex);
    charlistbox->clear();
    WorldSession *ws = instance->GetWSession();
    if(ws)
    {
        uint32 ffaction = racedb->GetFieldId("faction");
        for(uint32 i = 0; i < ws->GetCharsCount(); i++)
        {
            CharacterListExt& c = ws->GetCharFromList(i);
            core::stringw entry;
            entry += c.p._name.c_str();
            entry += L", ";
            entry += L"Level ";
            entry += c.p._level;
            entry += L" ";
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
}

void SceneCharSelection::OnUpdate(s32 timepassed)
{
    // treat doubleclick on listboxes as OK button click
    if(eventrecv->HasGUIEvent())
    {
        const SEvent::SGUIEvent& ev = eventrecv->NextGUIEvent();
        if(ev.EventType == EGET_LISTBOX_SELECTED_AGAIN)
        {
            if(ev.Caller == realmlistbox)
            {
                eventrecv->buttons |= BUTTON_REALMWIN_OK;
            }
            else if(ev.Caller == charlistbox)
            {
                eventrecv->buttons |= BUTTON_ENTER_WORLD;
            }
        }
        if(ev.EventType == EGET_ELEMENT_CLOSED)
        {
            if(ev.Caller == realmwin)//realmwin got closed via the close button, remove pointer
            {
                realmwin = NULL;
            }
            if(ev.Caller == newcharwin)//got closed via the close button, remove pointer
            {
                newcharwin = NULL;
            }
        }
        if(ev.EventType == EGET_COMBO_BOX_CHANGED)
        {
            if(ev.Caller == raceselect)
            {
                classselect->clear();
                u32 class_name = classdb->GetFieldId("name");
                u32 race_classmask = racedb->GetFieldId("classmask");
                u32 classmask = racedb->GetInt(racemap[raceselect->getSelected()],race_classmask);
                for(u32 i=1;i<=classdb->GetRowsCount();i++)
                {
                    if(classmask & 1<<i)//if class is in classmask, put it into the list
                    {
                        core::stringw name = classdb->GetString(i,class_name);
                        classmap[classselect->addItem(name.c_str())]=i;
                    }
                }

                
            }
        }
    }

    if(eventrecv->buttons & BUTTON_ENTER_WORLD && !realmwin && !newcharwin)
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
    if(eventrecv->buttons & BUTTON_BACK && !realmwin && !newcharwin) // cant cancel with any window open (important for ESC key handling)
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
    if(eventrecv->buttons & BUTTON_NEW_CHARACTER && !newcharwin)
    {
        dimension2d<s32> dim;
        rect<s32> pos;
        msgbox_textid = 0;
        newcharwin = guienv->addWindow(CalcRelativeScreenPos(driver, 0.2f, 0.2f, 0.6f, 0.6f), true,
            GetStringFromDB(ISCENE_CHARSEL_LABELS, DSCENE_CHARSEL_LABEL_NEWCHARWIN).c_str());
        pos = newcharwin->getAbsolutePosition(); // get absolute position and transform <dim> to absolute in-window position
        dim.Width = pos.LowerRightCorner.X - pos.UpperLeftCorner.X;
        dim.Height = pos.LowerRightCorner.Y - pos.UpperLeftCorner.Y;
        newcharwin->addChild(guienv->addButton(CalcRelativeScreenPos(dim, 0.7f, 0.93f, 0.12f, 0.05f), newcharwin, BUTTON_NEWCHARWIN_OK,
            GetStringFromDB(ISCENE_CHARSEL_BUTTONS, DSCENE_CHARSEL_NEWCHARWIN_OK).c_str()));
        newcharwin->addChild(guienv->addButton(CalcRelativeScreenPos(dim, 0.85f, 0.93f, 0.12f, 0.05f), newcharwin, BUTTON_NEWCHARWIN_CANCEL,
            GetStringFromDB(ISCENE_CHARSEL_BUTTONS, DSCENE_CHARSEL_NEWCHARWIN_CANCEL).c_str()));
        raceselect = guienv->addComboBox(CalcRelativeScreenPos(dim, 0.1f,0.1f,0.8f,0.05f), newcharwin);
        u32 race_name = racedb->GetFieldId("name");
        u32 race_classmask = racedb->GetFieldId("classmask");
        for(u32 i=1;i<=racedb->GetRowsCount();i++)
        {
            if(racedb->GetUint32(i,race_classmask)) //If the race has a classmask, it is playable
            {
                core::stringw name = racedb->GetString(i,race_name);
                racemap[raceselect->addItem(name.c_str())] = i;
            }
        }


        newcharwin->addChild(raceselect);
        classselect = guienv->addComboBox(CalcRelativeScreenPos(dim, 0.1f,0.2f,0.8f,0.05f), newcharwin);
        //newcharwin->addChild(classselect);
        guienv->addStaticText(L"Char Name", CalcRelativeScreenPos(dim,0.1f,0.3f,0.8f,0.05f),false,true,newcharwin);
        charname = guienv->addEditBox(L"", CalcRelativeScreenPos(dim,0.1f,0.35f,0.8f,0.05f),true, newcharwin);
        charname->setMax(12);
        msgbox = guienv->addStaticText(L"",CalcRelativeScreenPos(dim,0.2f,0.6f,0.6f,0.1f), true, true, newcharwin);
    }
    if(eventrecv->buttons & BUTTON_SELECT_REALM || scenedata[ISCENE_CHARSEL_REALMFIRST])
    {
        scenedata[ISCENE_CHARSEL_REALMFIRST] = 0;
        if(instance->GetRSession())
        {
            dimension2d<s32> dim;
            rect<s32> pos;
            realmwin = guienv->addWindow(CalcRelativeScreenPos(driver, 0.2f, 0.2f, 0.6f, 0.6f), true,
                GetStringFromDB(ISCENE_CHARSEL_LABELS, DSCENE_CHARSEL_LABEL_REALMWIN).c_str());
            pos = realmwin->getAbsolutePosition(); // get absolute position and transform <dim> to absolute in-window position
            dim.Width = pos.LowerRightCorner.X - pos.UpperLeftCorner.X;
            dim.Height = pos.LowerRightCorner.Y - pos.UpperLeftCorner.Y;
            realmwin->addChild(guienv->addButton(CalcRelativeScreenPos(dim, 0.7f, 0.93f, 0.12f, 0.05f), realmwin, BUTTON_REALMWIN_OK,
                GetStringFromDB(ISCENE_CHARSEL_BUTTONS, DSCENE_CHARSEL_REALMWIN_OK).c_str()));
            realmwin->addChild(guienv->addButton(CalcRelativeScreenPos(dim, 0.85f, 0.93f, 0.12f, 0.05f), realmwin, BUTTON_REALMWIN_CANCEL,
                GetStringFromDB(ISCENE_CHARSEL_BUTTONS, DSCENE_CHARSEL_REALMWIN_CANCEL).c_str()));
            realmlistbox = guienv->addListBox(CalcRelativeScreenPos(dim, 0.1f, 0.1f, 0.8f, 0.8f), realmwin);
            realmwin->addChild(realmlistbox);
            mutex.acquire();
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
            mutex.release();
        }
        else
        {
            guienv->addMessageBox(L"Not yet implemented!", L"This action is not yet supported.\nYou can change the realm only while still connected to the realm server.");
        }
    }    
    if(eventrecv->buttons & BUTTON_REALMWIN_OK && realmwin)
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

    if(eventrecv->buttons & BUTTON_NEWCHARWIN_OK && newcharwin)
    {
        core::stringc chname = charname->getText();
        u8 race = racemap[raceselect->getSelected()];
        u8 cclass = classmap[classselect->getSelected()];
        if(chname.size() && race && cclass)
        {
            WorldSession *ws=instance->GetWSession();
            if(ws)
            {
                ws->SendCharCreate(chname.c_str(), race, cclass);

                msgbox->setText(GetStringFromDB(3,0).c_str());
                msgbox_textid = 0;

                // do not close window until character created (will when getting response code 0)
            }
            else
                logerror("Trying to create new Character, but no WorldSession exists.");
        }
        else
            logerror("Race, Class or Name not set!");
    }

    // realmlist window
    if(eventrecv->buttons & BUTTON_REALMWIN_CANCEL && realmwin)
    {
        realmwin->remove();
        realmwin = NULL;
    }

    // new character window
    if(eventrecv->buttons & BUTTON_NEWCHARWIN_CANCEL && newcharwin)
    {
        newcharwin->remove();
        newcharwin = NULL;
    }
    
    if(newcharwin && msgbox_textid != scenedata[ISCENE_CHARSEL_ERRMSG])
    {
        msgbox_textid = scenedata[ISCENE_CHARSEL_ERRMSG];
        if(SCPDatabase *generictext = instance->dbmgr.GetDB("generic_text"))
        {
            msgbox->setText(GetStringFromDB(0, msgbox_textid, generictext).c_str());
        }
        if(scenedata[ISCENE_CHARSEL_ERRMSG] == CHAR_CREATE_SUCCESS)
        {
            newcharwin->remove();
            newcharwin = NULL;
        }
    }

    eventrecv->buttons = 0;
}

void SceneCharSelection::OnDelete(void)
{
}


void SceneCharSelection::OnResize(void)
{
//TODO: Handle Resizes correctly. This goes for the loginscreen as well

}

// called when receiving SMSG_CHAR_ENUM
void SceneCharSelection::OnManualUpdate(void)
{
    Scene::OnManualUpdate();
    FillCharlist();
}

