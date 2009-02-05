/* 
 * Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MANGOS_OBJECTDEFINES_H
#define MANGOS_OBJECTDEFINES_H

enum HighGuid
{
 	HIGHGUID_ITEM           = 0x4000,                       // blizz 4000 
 	HIGHGUID_CONTAINER      = 0x4000,                       // blizz 4000 
 	HIGHGUID_PLAYER         = 0x0000,                       // blizz 0000 
 	HIGHGUID_GAMEOBJECT     = 0xF110,                       // blizz F110 
 	HIGHGUID_TRANSPORT      = 0xF120,                       // blizz F120 (for GAMEOBJECT_TYPE_TRANSPORT) 
 	HIGHGUID_UNIT           = 0xF130,                       // blizz F130 
 	HIGHGUID_PET            = 0xF140,                       // blizz F140 
 	HIGHGUID_VEHICLE        = 0xF150,
 	HIGHGUID_DYNAMICOBJECT  = 0xF100,                       // blizz F100 
 	HIGHGUID_CORPSE         = 0xF101,                       // blizz F100 
 	HIGHGUID_MO_TRANSPORT   = 0x1FC0,                       // blizz 1FC0 (for GAMEOBJECT_TYPE_MO_TRANSPORT) 
};

#define IS_CREATURE_GUID(Guid)       ( GUID_HIPART(Guid) == HIGHGUID_UNIT )
#define IS_PLAYER_GUID(Guid)         ( GUID_HIPART(Guid) == HIGHGUID_PLAYER )
#define IS_ITEM_GUID(Guid)           ( GUID_HIPART(Guid) == HIGHGUID_ITEM )
#define IS_GAMEOBJECT_GUID(Guid)     ( GUID_HIPART(Guid) == HIGHGUID_GAMEOBJECT )
#define IS_DYNAMICOBJECT_GUID(Guid)  ( GUID_HIPART(Guid) == HIGHGUID_DYNAMICOBJECT )
#define IS_CORPSE_GUID(Guid)         ( GUID_HIPART(Guid) == HIGHGUID_CORPSE )
#define IS_PLAYER_CORPSE_GUID(Guid)  ( GUID_HIPART(Guid) == HIGHGUID_PLAYER_CORPSE )
#define IS_TRANSPORT(Guid)           ( GUID_HIPART(Guid) == HIGHGUID_TRANSPORT )
#define IS_VEHICLE_GUID(Guid)        ( GUID_HIPART(Guid) == HIGHGUID_VEHICLE )
#endif
