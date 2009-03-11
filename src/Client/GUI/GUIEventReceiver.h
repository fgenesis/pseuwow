#ifndef GUIEVENTRECEIVER_H
#define GUIEVENTRECEIVER_H


class GUIEventReceiver : public IEventReceiver
{
public:
    GUIEventReceiver()
    {
        buttons = 0;
        react_to_keys = true;
        memset(&guievent, 0 , sizeof(SEvent));
        memset(&keyevent, 0 , sizeof(SEvent));
        memset(&mouseevent, 0 , sizeof(SEvent));
        guievent_proc = false;
        mouseevent_proc = false;
        keyevent_proc = false;
    }
    virtual bool OnEvent(const SEvent& event)
    {
        // copy all 3 event types into different stores for later external use
        if(event.EventType == EET_GUI_EVENT)
        {
            guievent = event;
            guievent_proc = false;
        }
        else if(event.EventType == EET_KEY_INPUT_EVENT)
        {
            keyevent = event;
            keyevent_proc = false;
        }
        else if(event.EventType == EET_MOUSE_INPUT_EVENT)
        {
            mouseevent = event;
            mouseevent_proc = false;
        }

        bool proc = false;
        //GUI EVENT
        if (event.EventType == EET_GUI_EVENT)
        {
            s32 id = event.GUIEvent.Caller->getID();

            DEBUG(logdev("GUIEventReceiver: event type %u ID %u",event.GUIEvent.EventType,id));

            switch(event.GUIEvent.EventType)
            {
            case EGET_BUTTON_CLICKED:
                buttons += id;
                proc = true;
                break;

            case EGET_MESSAGEBOX_OK: // triggered on enter or ok button click
            case EGET_MESSAGEBOX_CANCEL: // triggered on escape
                react_to_keys = true; // popup is gone, main window can react to keys again
                proc = true;
                break;
            }

            if(customHandledEvents.find(event.GUIEvent.EventType) != customHandledEvents.end())
                proc = true;

        }

        if(react_to_keys && event.EventType == EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown)
        {
            std::map<u32,u32>::iterator it = keyToButtonMap.find(event.KeyInput.Key);
            if( it != keyToButtonMap.end() )
            {
                buttons += it->second;
                proc = true;
            }
        }
        return proc;
    }

    bool react_to_keys;
    u32 buttons;
    SEvent guievent, mouseevent, keyevent;
    bool keyevent_proc, mouseevent_proc, guievent_proc;
    std::map<u32,u32> keyToButtonMap; // to simulate button press on key input
    std::set<u32> customHandledEvents;

};

#endif


