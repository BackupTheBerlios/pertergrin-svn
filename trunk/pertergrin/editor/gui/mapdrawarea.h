/* mapdrawarea.h - Pertergrin Editor class declaration of the map draw area
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

#ifndef MAPDRAWAREA_H
#define MAPDRAWAREA_H

// includes
#include <gtkmm.h>
#include <glibmm.h>
#include <vector>

using namespace std;

// Definition of a tile
typedef struct cw_tile_pos
{
  Glib::ustring   oName;             /* Name of the graphic tile        */
  int             iXPos;             /* X-Position inside tiles graphic */
  int             iYPos;             /* Y-Position inside tiles graphic */
  int          iTileset;             /* Tileset to be used              */
} cwtpos_t;

typedef struct cw_tile_map
{
  int                        iXSize;   /* Number of Columns in the map  */
  int                        iYSize;   /* Number of Rows in the map     */
  vector< vector<cwtpos_t> > &vecoMap; /* Stores the graphic tiles      */
  int                    iXTileSize;   /* X Size of a tile              */
  int                    iYTileSize;   /* Y Size of a tile              */
} cwtmap_t;

/// Custom Widget Map (with Tiles) Draw Area
/// To use it you need to have one or several tile sets.
/// You need to have a map which defines where the tiles are places
/// You also need to load or create the pixbuf(s) being used as tiles
/// You don't need to define the whole map at once
class CWMapDrawArea : public Gtk::DrawingArea
{
public:
  CWMapDrawArea(Glib::RefPtr<Gdk::Pixbuf> &opTileset, 

              int &iXTileSize, int &iYTileSize);
  void setMap(cwtmap_t &oTilesMap, int iXSize=0, int iYSize=0);
  void changeMap(int iWidth, int iHeight, int iXPos=0, int iYPos=0);
  void changeMap(int iWidth, int iHeight, int iXPos, int iYPos, 
                 cwtpos_t &oTile);
  void setDefaultTile(cwtpos_t &oTile);
  void setTilesetsSize(int iNum);
  void addTileset(Glib::RefPtr<Gdk::Pixbuf> &opTileset, int iPos);
  void removeTileset(int iPos);
  void clearTilesets();

protected:
  bool on_map_area_expose (GdkEventExpose *event);
  bool on_map_area_value_changed();
  void on_set_scroll_adjustments(Gtk::Adjustment *opHadjustment,
                                 Gtk::Adjustment *opVadjustment);

  void adjust_scrollbars (void);

private:
  Glib::RefPtr<Gdk::Pixbuf> &mopDefaultTileset;
  Glib::RefPtr<Gdk::Pixbuf> moDrawPB;
  Gtk::ScrolledWindow       moScrollPane;
  Gtk::DrawingArea          moDrawingArea;
  Gtk::Viewport             *mopParent;
  vector< Glib::RefPtr<Gdk::Pixbuf> > mopTilesets;
  cwtmap_t moTilesMap;
  cwtpos_t moDefaultTile;
  int miDrawAreaX;
  int miDrawAreaY;
};

#endif // MAPDRAWAREA_H
