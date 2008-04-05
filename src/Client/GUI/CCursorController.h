// Copyright (c) 2007-2008 Tomer Nosrati
// This file is part of the "NUSoftware Game Engine".
// For conditions of distribution and use, see copyright notice in nge.h

#ifndef __C_CURSOR_H__
#define __C_CURSOR_H__

#include "irrlicht/irrlicht.h"

using namespace irr;
using namespace irr::core;
using namespace irr::gui;
using namespace irr::scene;
using namespace irr::video;

//! Cursor controller v0.2
/*! This class controls the cursor. You can choose to use the OS's cursor, a custom
cursor or both. You can also have many custom cursors and switch between them easily.
*/
class CCursorController
{
    struct TexBoolPair
    {
        TexBoolPair(ITexture *t, bool tl) { tex = t; topleft = tl; }
        ITexture *tex;
        bool topleft;
    };
public:
   CCursorController(ICursorControl* irrCursor, IVideoDriver* irrVideoDriver);
   ~CCursorController();

   void setVisible(bool visible);
   bool isVisible() const;
   void setOSCursorVisible(bool visible);
   bool isOSCursorVisible() const;

   void render();
   void addMouseCursorTexture(c8* Cursor_file, bool top_left = false);
   ITexture* getCursorTexture(u32 index) const;
   void removeCursor(u32 index);
   void setActiveCursor(u32 index);
   void Clear();
   position2di& getMousePos();

private:
   position2di& updateMousePos();

   bool IsOSCursorVisible;
   bool IsVisible;
   bool topleft;
   position2di m_MousePos;
   ICursorControl* irrCursorControl;
   IVideoDriver* videoDriver;
   array<TexBoolPair> m_aMouseCursors;
   ITexture* m_pMouseCursor;

   u32 used;
};
#endif // __C_CURSOR_H__
