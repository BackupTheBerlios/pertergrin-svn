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
PtgWorld::PtgWorld(int iXSize, int iYSize, Glib::ustring oName, Glib::ustring oWorldType,
                   int iTileWidth, int iTileHeight, int iRegionXSize, int iRegionYSize)
{
  msWorldData.oName = "";
  msWorldData.oDescription = "";
  msWorldData.oWorldType = oWorldType;
  msWorldData.iXSize = 0;
  msWorldData.iYSize = 0;
  msWorldData.oStartDate = "";
  msWorldData.iRegionX = 0;
  msWorldData.iRegionY = 0;
  msWorldData.iRegionXSize = 0;
  msWorldData.iRegionYSize = 0;	
  msWorldData.iTileWidth = 0;
  msWorldData.iTileHeight = 0;
  
  // Default selected region (nothing selected)
  mapClearRegion();
  
  setSize(iXSize, iYSize);
  setRegionSize(iRegionXSize, iRegionYSize);
  setName(oName);

  // Prevent too small graphic tiles
  if( iTileWidth < 10 )
    iTileWidth = 10;
  if( iTileHeight < 10 )
    iTileHeight = 10;

  msWorldData.iTileWidth = iTileWidth;
  msWorldData.iTileHeight = iTileHeight;

}

// Clears the map
// Do we really need this method ?
void PtgWorld::mapClear(int iXPos, int iYPos, int iWidth, int iHeight)
{
  mapFill(iXPos, iYPos, iWidth, iHeight, 
          msClearDefaultPart);
}

// Clears the map
// Do we really need this method ?
void PtgWorld::mapClear(int iWidth, int iHeight)
{
  mapFill(msWorldData.iRegionX, msWorldData.iRegionY, iWidth, iHeight, 
          msClearDefaultPart);
}

// Clears the map
// Do we really need this method ?
void PtgWorld::mapClear()
{
  mapFill(msWorldData.iRegionX, msWorldData.iRegionY,
          msWorldData.iRegionXSize, msWorldData.iRegionYSize, msClearDefaultPart);
}

// Sets a landscape part at a given position
// Input parameters:
// iXPos  - X-Position of the landscape map part
// iYPos  - Y-Position of the landscape map part
// sLPart  -  The landscape map part at position iXPos, iYPos
void PtgWorld::mapSet(int iXPos, int iYPos, plpart_t sLPart)
{
  // Get reference to row of the world map
  msWorldData.vecoWorldMap[iXPos+iYPos*msWorldData.iXSize] = sLPart;
}

// Get a landscape map part from a given position
// Input parameters:
// iXPos  - X-Position of the landscape map part
// iYPos  - Y-Position of the landscape map part
// Return parameter:
// The landscape map part at position iXPos, iYPos
plpart_t &PtgWorld::mapGet(int iXPos, int iYPos)
{
  // Get copy of row of the world map (so map cannot be directly modified)
  return msWorldData.vecoWorldMap[iXPos+iYPos*msWorldData.iXSize];
}

void PtgWorld::mapMove(int iDestXPos, int iDestYPos, int iSourceXPos, int SourceYPos, 
                       int iWidth, int iHeight)
{
}

// Fills the specified region with a map piece
// Warning: Will remove any edited map parts in the region!!!
// Input parameters:
// iX  -  X Position of the region
// iY  -  Y Position of the region
// iWidth  -  Width of the region
// iHeigth  -  Height of the region
// sLPart  -  Landscape map part to be set in the region
void PtgWorld::mapFill(int iXPos, int iYPos, int iWidth, int iHeight, plpart_t sLPart)
{
  int iXC, iYC;
  // Make sure the selection is not outside of the map
  if( (iXPos + iWidth) > msWorldData.iXSize )
    iWidth = msWorldData.iXSize - iXPos;
  if( (iYPos + iHeight) > msWorldData.iYSize )
    iWidth = msWorldData.iYSize - iYPos;
  for( iXC=iXPos; iXC < iWidth; iXC++ )
  {
    for( iYC=iYPos; iYC < iHeight; iYC++ )
    {
      // Copy the landscape part data
      msWorldData.vecoWorldMap[iXC+iYC*msWorldData.iXSize] = sLPart;
    }
  }
}

// Fills the specified (selected) region with a map piece
// Warning: Will remove any edited map parts in the region!!!
// Input parameters:
// iWidth  -  Width of the region
// iHeigth  -  Height of the region
// sLPart  -  Landscape map part to be set in the region
void PtgWorld::mapFill(int iWidth, int iHeight, plpart_t sLPart)
{
  mapFill(msWorldData.iRegionX, msWorldData.iRegionY, iWidth, iHeight, sLPart);
}

// Fills the selected region with a map piece
// Warning: Will remove any edited map parts in the region!!!
// Input parameters:
// sLPart  -  Landscape map part to be set in the region
void PtgWorld::mapFill(plpart_t sLPart)
{
  mapFill(msWorldData.iRegionX, msWorldData.iRegionY,
          msWorldData.iRegionXSize, msWorldData.iRegionYSize, sLPart);
}

// Set the map default part to clear parts of the world map
// Input parameters:
// sLPart - Clear default world map part
void PtgWorld::mapSetClearDefault(plpart_t sLPart)
{
  msClearDefaultPart = sLPart;
}

// Select a region inside the world map
// Input parameters:
// iX  -  Selected region column
// iY  -  Selected region row
// iXSize  -  Number of Columns in the region
// iYSize  -  Number of Rows in the region
void PtgWorld::mapSelect(int iX, int iY, int iXSize, int iYSize)
{
  // Selected region has to be inside map
  if( iX < 0 )
    iX = 0;
  if( iY < 0 )
    iY = 0;
  if( iXSize < 1 )
    iXSize = 1;
  if( iYSize < 1 )
    iYSize = 1;

  msWorldData.iRegionX = iX;
  msWorldData.iRegionY = iY;
  msWorldData.iRegionXSize = iXSize;
  msWorldData.iRegionYSize = iYSize;
}

// Removes any selection inside the map
void PtgWorld::mapClearRegion()
{
  msWorldData.iRegionX = 0;
  msWorldData.iRegionY = 0;
  msWorldData.iRegionXSize = 0;
  msWorldData.iRegionYSize = 0;
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

  msWorldData.iXSize = iXSize;
  msWorldData.iYSize = iYSize;
  // Reserve size so vectors do not reallocate all the time
  // Attention: for real size maps > 1000 you may not use the complete
  // size but only the size which fits in memory!
  msWorldData.vecoWorldMap.reserve(iXSize*iYSize);
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

// Set the region size of the world
// Input parameters:
// iRegionXSize  -  Region X-Size of the world
// iRegionYSize  -  Region Y-Size of the world
void PtgWorld::setRegionSize(int iRegionXSize, int iRegionYSize)
{
  // Max. region size is size of map
  if( iRegionXSize > msWorldData.iXSize )
    iRegionXSize = msWorldData.iXSize;
  if( iRegionYSize > msWorldData.iYSize )
    iRegionYSize = msWorldData.iYSize;

  // Prevent a too small world map
  if( iRegionXSize < 10 )
    iRegionXSize = 10;
  if( iRegionYSize < 10 )
    iRegionYSize = 10;
  
  msWorldData.iRegionXSize = iRegionXSize;
  msWorldData.iRegionYSize = iRegionYSize;
}

// Get the region size of the world  
// Output parameters:
// iRegionXSize  -  Region X-Size of the world
// iRegionYSize  -  Region Y-Size of the world
void PtgWorld::getRegionSize(int &iRegionXSize, int &iRegionYSize)
{
  iRegionXSize = msWorldData.iRegionXSize;
  iRegionYSize = msWorldData.iRegionYSize;
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
  iWidth = msWorldData.iTileWidth;
  iHeight = msWorldData.iTileHeight;
}
