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
  : mopDefaultTileset( opTileset )
{
  // First pixbuf
  mopTilesets[0] = opTileset;
  moTilesMap.iXTileSize = iXTileSize;
  moTilesMap.iXTileSize = iYTileSize;
  // Connect to expose event
  signal_expose_event().connect( slot(*this, 
                                 &CWMapDrawArea::on_map_area_expose) );
  // Fetch parent viewport
  mopParent = (Gtk::Viewport *) get_parent();
  g_assert( mopParent != 0 );
  mopParent->signal_set_scroll_adjustments().connect( slot(*this, 
			   &CWMapDrawArea::on_set_scroll_adjustments) );
  adjust_scrollbars();
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

  moTilesMap.iXSize = iXSize;
  moTilesMap.iYSize = iYSize;
  // Reserve size so vectors do not reallocate all the time
  // Attention: for real size maps > 1000 you may not use the complete
  // size but only the size which fits in memory!
  moTilesMap.vecoMap.reserve(iXSize);
  for(int i=0; i<iXSize; i++)
  {
    // Copy part of the map
    moTilesMap.vecoMap[i] = oTilesMap.vecoMap[i];
  }  
  //moTilesMap = oTilesMap;
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
    vector<cwtpos_t> &vecoRow = moTilesMap.vecoMap[iXC];
    for( iYC=iYPos; iYC < iHeight; iYC++ )
      // Copy the tile data
      vecoRow[iYC] = oTile;
  }
}

/// Sets the default tile 
/// oTile  -  Tile to be set as default
void CWMapDrawArea::setDefaultTile(cwtpos_t &oTile)
{
  moDefaultTile = oTile;
}

/// Sets the size of the tileset vector
/// iNum  -  Number of tilesets in the vector
void CWMapDrawArea::setTilesetsSize(int iNum)
{
  mopTilesets.reserve(iNum);
}

/// Adds a tileset to the tileset vector
/// opTileset  -  New tileset
/// iPos  -  Position in the tilesets
void CWMapDrawArea::addTileset(Glib::RefPtr<Gdk::Pixbuf> &opTileset, int iPos)
{
  // Set tileset (size check ?)
  mopTilesets[iPos] = opTileset;
}

/// Removes a tileset from the tileset vector
/// iPos  -  Position of the tileset to be removed
void CWMapDrawArea::removeTileset(int iPos)
{
  mopTilesets.erase(mopTilesets.begin()+iPos);
}

/// Removes all tilesets from the tileset vector
void CWMapDrawArea::clearTilesets()
{
  mopTilesets.clear();
}

/// Draws the map area on the screen
/// event  -  event data
bool CWMapDrawArea::on_map_area_expose (GdkEventExpose *event)
{
  int iWidth = moDrawPB->get_width();
  int iHeight = moDrawPB->get_height();

  const Glib::RefPtr<Gdk::Drawable> oWindow = get_window();
  const Glib::RefPtr<Gdk::GC> oGC = get_style()->get_fg_gc(Gtk::STATE_NORMAL);
  // Draw part of the map not having been seen
  // Draw complete pixbuf to screen
  moDrawPB->render_to_drawable( oWindow, oGC, 0, 0, 0, 0, iWidth, iHeight, 
                                Gdk::RGB_DITHER_NORMAL, iWidth, iHeight);
  return true;
}

bool CWMapDrawArea::on_map_area_value_changed()
{
  return false;
}

void CWMapDrawArea::on_set_scroll_adjustments(Gtk::Adjustment *opHadjustment,
                                              Gtk::Adjustment *opVadjustment)
{
  int iX1 = 0, iY1 = 0, iX2 = 0, iY2 = 0, iWidth, iHeight, iX=0, iY=0;

  // Size of drawarea
  iWidth = get_width();
  iHeight = get_height();

  // Get Position of adjustments
  if( opHadjustment->get_lower() != ( opHadjustment->get_upper() - 
                                      opHadjustment->get_page_size() ) )
  {
    iX = opHadjustment->get_value() - opHadjustment->get_lower();
  }
  if( opVadjustment->get_lower() != ( opVadjustment->get_upper() - 
                                      opVadjustment->get_page_size() ) )
  {
    iY = opVadjustment->get_value() - opVadjustment->get_lower();
  }

  // Check the X direction being moved
  if ( miDrawAreaX - iX > iWidth )
    iX2 = miDrawAreaX - iX; // positive move
  else if( iX - miDrawAreaX > iWidth )
    iX1 = iX - miDrawAreaX; // negative move
  
  // Check the Y direction being moved
  if ( miDrawAreaY - iY > iHeight )
    iY2 = miDrawAreaY - iY; // positive move
  else if( iY - miDrawAreaY > iHeight )
    iY1 = iY - miDrawAreaY; // negative move

  // Move part of the map being visible
  const Glib::RefPtr<Gdk::GC> oGC = get_style()->get_fg_gc(Gtk::STATE_NORMAL);
  const Glib::RefPtr<Gdk::Drawable> oWindow = get_window();
    oWindow->draw_drawable(oGC, oWindow, iX1, iY1, iX2, iY2, iWidth, iHeight );

  // Calculate position inside map using size of tiles
  iX = iX / moTilesMap.iXTileSize;
  iY = iY / moTilesMap.iYTileSize;

  iWidth = iWidth / moTilesMap.iXTileSize + 1;
  iHeight = iHeight / moTilesMap.iYTileSize + 1;
  // Draw the rest of the map
  for( iX1 = iX; iX < iHeight; iX1++ )
  {
    vector<cwtpos_t> &vecoRow = moTilesMap.vecoMap[iX1];
    for( iY1 = iY; iY < iHeight; iY1++ )
    {
      // Copy map tile to draw buffer (double buffering)
      mopTilesets[vecoRow[iY1].iTileset]->copy_area(vecoRow[iY1].iXPos,
                                                    vecoRow[iY1].iYPos, 
                                                    moTilesMap.iXTileSize,
                                                    moTilesMap.iYTileSize,
                                                    moDrawPB,
                                                    iX1*moTilesMap.iXTileSize,
                                                    iY1*moTilesMap.iYTileSize);
    }
  }
  moDrawPB->render_to_drawable( oWindow, oGC, 0, 0, 0, 0, iWidth, iHeight, 
                                Gdk::RGB_DITHER_NORMAL, iWidth, iHeight);

  miDrawAreaX = iX;
  miDrawAreaY = iY;
}

void CWMapDrawArea::adjust_scrollbars(void)
{
}

