#ifndef GUIEVENTRECEIVER_H
#define GUIEVENTRECEIVER_H

#include <queue>


class GUIEventReceiver : public IEventReceiver
{
public:
    GUIEventReceiver()
    {
        buttons = 0;
        react_to_keys = true;
        store_gui = true;
        store_keys = true;
        store_mouse = true;
    }
    virtual bool OnEvent(const SEvent& event)
    {
        // copy all 3 event types into different stores for later external use
        if(event.EventType == EET_GUI_EVENT)
        {
            guieventqueue.push_back(event.GUIEvent);
        }
        else if(event.EventType == EET_KEY_INPUT_EVENT)
        {
            keyeventqueue.push_back(event.KeyInput);
        }
        else if(event.EventType == EET_MOUSE_INPUT_EVENT)
        {
            mouseeventqueue.push_back(event.MouseInput);
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

    inline bool HasMouseEvent(void) { return mouseeventqueue.size(); }
    inline bool HasGUIEvent(void) { return guieventqueue.size(); }
    inline bool HasKeyEvent(void) { return keyeventqueue.size(); }


    inline SEvent::SMouseInput NextMouseEvent(void)
    {
        ASSERT(HasMouseEvent())
        const SEvent::SMouseInput ev = mouseeventqueue.front();
        mouseeventqueue.pop_front();
        return ev;
    }

    inline SEvent::SGUIEvent NextGUIEvent(void)
    {
        ASSERT(HasGUIEvent())
        SEvent::SGUIEvent ev = guieventqueue.front();
        guieventqueue.pop_front();
        return ev;
    }

    inline SEvent::SKeyInput NextKeyEvent(void)
    {
        ASSERT(HasKeyEvent())
        SEvent::SKeyInput ev = keyeventqueue.front();
        keyeventqueue.pop_front();
        return ev;
    }

    bool react_to_keys, store_mouse, store_keys, store_gui;
    u32 buttons;
    std::map<u32,u32> keyToButtonMap; // to simulate button press on key input
    std::set<u32> customHandledEvents;

protected:
    std::list<SEvent::SGUIEvent> guieventqueue;
    std::list<SEvent::SMouseInput> mouseeventqueue;
    std::list<SEvent::SKeyInput> keyeventqueue;

};

#endif


