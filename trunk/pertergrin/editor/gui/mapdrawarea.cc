/* mainwindow.cc - Pertergrin Editor class definition of the map draw area
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

#include "mapdrawarea.h"

// This should be removed ASAP
// This class probably will never know anything about configuration
#define PTG_PROTOTYPE 1

/// Default-Konstruktor
/// Input Parameters:
/// opTileset  -  Default Pixbuf for drawing the map
/// iXTileSize  -  size of a tile graphic
/// iXTileSize  -  Y size of a tile graphic
CWMapDrawArea::CWMapDrawArea(Glib::RefPtr<Gdk::Pixbuf> &opTileset, 
                             int &iXTileSize, int &iYTileSize)
  : mopDefaultTileset( opTileset ), miXTileSize( iXTileSize ), 
    miYTileSize( iYTileSize )
{
  // First pixbuf
  mopTilesets[0] = opTileset;
}

/// Set the complete map
/// Input Parameters:
/// oTilesMap  -  Complete map to be drawn (will be copied!)
/// iXSize  -  X size of the map
/// iYSize  -  Y size of the map
void CWMapDrawArea::setMap(cwtmap_t &oTilesMap, int iXSize, int iYSize)
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

  iMapXSize = iXSize;
  iMapYSize = iYSize;
  // Reserve size so vectors do not reallocate all the time
  // Attention: for real size maps > 1000 you may not use the complete
  // size but only the size which fits in memory!
  moTilesMap.vecoColumns.reserve(iXSize);
  for(int i=0; i<iXSize; i++)
  {
    moTilesMap.vecoRows = moTilesMap.vecoColumns[i];
    moTilesMap.vecoRows.reserve(iYSize);
  }  
  moTilesMap = oTilesMap;
}

void CWMapDrawArea::changeMap(int iWidth, int iHeight, int iXPos, int iYPos)
{
  changeMap( iWidth, iHeight, iXPos, iYPos, moDefaultTile );
}

/// Changes a part map
/// Input Parameters:
/// iWidth  -  Width of the map part
/// iHeight  -  Height of the map part
/// iXPos  -  X Position in the map
/// iYPos  -  Y Position in the map
/// oTile  -  Tile used to draw the map part
void CWMapDrawArea::changeMap(int iWidth, int iHeight, int iXPos, int iYPos,
                              cwtpos_t &oTile)
{
  int iXC, iYC;
  for( iXC=iXPos; iXC<iWidth; iXC++ )
  {
    moTilesMap.vecoRows = moTilesMap.vecoColumns[iXC];
    for( iYC=iYPos; iYC < iHeight; iYC++ )
      // Copy the tile data
      moTilesMap.vecoRows[iYC] = oTile;
  }
}

/// Sets the default tile 
/// oTile  -  Tile to be set as default
void CWMapDrawArea::setDefaultTile(cwtpos_t &oTile)
{
  moDefaultTile = oTile;
}

/// Sets the size of the tilesets
/// iNum  -  Number of tilesets
void CWMapDrawArea::setTilesetsSize(int iNum)
{
  mopTilesets.reserve(iNum);
}

/// Adds a tileset to the tilesets
/// opTileset  -  New tileset
/// iPos  -  Position in the tilesets
void CWMapDrawArea::addTileset(Glib::RefPtr<Gdk::Pixbuf> &opTileset, int iPos)
{
  // Set tileset (size check ?)
  mopTilesets[iPos] = opTileset;
}

void CWMapDrawArea::removeTileset(Glib::RefPtr<Gdk::Pixbuf> &opTileset, 
                                  int iPos)
{
  mopTilesets.erase(mopTilesets.begin()+iPos);
}
  
void CWMapDrawArea::clearTilesets()
{
  mopTilesets.clear();
}

bool CWMapDrawArea::on_map_area_expose (GdkEventExpose *event)
{
  return false;
}

bool CWMapDrawArea::on_map_area_value_changed()
{
  return false;
}

void CWMapDrawArea::adjust_scrollbars(void)
{
}

