/* world.cc - Pertergrin Editor class definition of the world management
 *
 * Copyright (C) 2001-2003 Pertergrin development team
 *
 * It is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License,  or (at your option) any later
 * version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not contact one the AUTHORS.
 */

#include "world.h"

// Empty default constructor
PtgWorld::PtgWorld(int iXSize, int iYSize, Glib::ustring oName, 
                   int iTileWidth, int iTileHeight)
{
  setSize(iXSize, iYSize);
  setName(oName);

  // Prevent too small graphic tiles
  if( iTileWidth < 10 )
    iTileWidth = 10;
  if( iTileHeight < 10 )
    iTileHeight = 10;

  msWorldData.iWidth = iTileWidth;
  msWorldData.iHeight = iTileHeight;
}

void PtgWorld::mapClear(int iXPos, int iYPos, int iWidth, int iHeight)
{
}

void PtgWorld::mapClear(int iWidth, int iHeight)
{
}

void PtgWorld::mapSet(int iXPos, int iYPos, plpart_t sLPart)
{
}

// Get a world map part
// Input parameters:
// iXPos  - X-Position of the world map part
// iYPos  - Y-Position of the world map part
// Return parameter:
// The world map part at position iXPos, iYPos
plpart_t &PtgWorld::mapGet(int iXPos, int iYPos)
{
  msWorldData.vecoRows = msWorldData.vecoColumns[iXPos];
  return msWorldData.vecoRows[iYPos];
}

void PtgWorld::mapMove(int iDestXPos, int iDestYPos, int iSourceXPos, int SourceYPos, 
                       int iWidth, int iHeight)
{
}

void PtgWorld::mapFill(int iXPos, int iYPos, int iWidth, int iHeight, plpart_t sLPart)
{
}

void PtgWorld::mapFill(int iWidth, int iHeight, plpart_t sLPart)
{
}

// Set the map default part to clear parts of the world map
// Input parameters:
// sLPart - Clear default world map part
void PtgWorld::mapSetClearDefault(plpart_t sLPart)
{
  msClearDefaultPart = sLPart;
}

// Set the size of the world
// Input Parameters:
// iXSize  -  X-Size of the world
// iYSize  -  Y-Size of the world
void PtgWorld::setSize(int iXSize, int iYSize)
{
#ifdef PTG_PROTOTYPE
  // Currently prevent too large Maps
  if( iXSize > 1000 )
    iXSize = 1000;
  if( iYSize > 1000 )
    iYSize = 1000;
#endif

  // Prevent a too small world map
  if( iXSize < 10 )
    iXSize = 10;
  if( iYSize < 10 )
    iYSize = 10;

  iXSize = msWorldData.iXSize;
  iYSize = msWorldData.iYSize;
}

// Get the size of the world
// Output parameters:
// iXSize  -  X-Size of the world
// iYSize  -  Y-Size of the world
void PtgWorld::getSize(int &iXSize, int &iYSize)
{
  iXSize = msWorldData.iXSize;
  iYSize = msWorldData.iYSize;
}

// Set the name of the world
// Input Parameters:
// oName  -  Name of the world
void PtgWorld::setName(Glib::ustring oName)
{
  msWorldData.oName = oName;
}

// Get the name of the world
// Return parameter:
// Name of the world
Glib::ustring &PtgWorld::getName()
{
  return msWorldData.oName;
}

// Set the description of the world
// Input Parameters:
// oDescription  -  Short description of the world
void PtgWorld::setDescription(Glib::ustring oDescription)
{
  msWorldData.oDescription = oDescription;
}

// Get the description of the world
// Return parameter:
// Short description of the world
Glib::ustring &PtgWorld::getDescription()
{
  return msWorldData.oDescription;
}

// Get the whole world data
// Return parameter:
// World data itself (better would be a copy, but that costs too much memory)
// Do we really need that method ?
pworld_t &PtgWorld::getWorldData()
{
  return msWorldData;
}

// Get the world type (aka Pertergrin)
// Return parameter:
// World type for this world
Glib::ustring &PtgWorld::getWorldType()
{
  return msWorldData.oWorldType;
}

// Get the tile size used to draw a map part
// Output parameters:
// iWidth  -  Width of all the graphic tiles
// iHeight  -  Height of all the graphic tiles
void PtgWorld::getTileSize(int &iWidth, int &iHeight)
{
  iWidth = msWorldData.iWidth;
  iHeight = msWorldData.iHeight;
}
