/* world.h - Pertergrin Editor class declaration of the world management
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

#ifndef PTGWORLD_H
#define PTGWORLD_H

#include "ptgtypes.h"

class PtgWorld
{
public:
  PtgWorld(int iXSize, int iYSize, Glib::ustring oName, Glib::ustring oWorldType,
           int iTileWidth, int iTileHeight, int iRegionXSize, int iRegionYSize);

  // Methods changing the map directly
  void mapClear(int iXPos, int iYPos, int iWidth, int iHeight);
  void mapClear(int iWidth, int iHeight);
  void mapClear();
  void mapSet(int iXPos, int iYPos, plpart_t sLPart);
  plpart_t &mapGet(int iXPos, int iYPos);
  void mapMove(int iDestXPos, int iDestYPos, int iSourceXPos, int SourceYPos, 
               int iWidth=1, int iHeight=1);
  void mapFill(int iXPos, int iYPos, int iWidth, int iHeight, plpart_t sLPart);
  void mapFill(int iWidth, int iHeight, plpart_t sLPart);
  void mapFill(plpart_t sLPart);
  void mapSetClearDefault(plpart_t sLPart);
  void mapSelect(int iX, int iY, int iXSize, int iYSize);
  void mapClearRegion();
  //void mapUndo();
  //void mapRedo();

  // Methods changing the world
  void setSize(int iXSize, int iYSize);
  void getSize(int &iXSize, int &iYSize);
  void setRegionSize(int iRegionXSize, int iRegionYSize);
  void getRegionSize(int &iRegionXSize, int &iRegionYSize);
  void setName(Glib::ustring oName);
  Glib::ustring &getName();
  void setDescription(Glib::ustring oDescription);
  Glib::ustring &getDescription();
  //setWorldType(pworldtype_t eType);
  //setStartDate(Glib::ustring oStartDate);

  // Read-Only methods
  pworld_t &getWorldData();
  Glib::ustring &getWorldType();
  void getTileSize(int &iWidth, int &iHeight);

private:
  pworld_t msWorldData;
  plpart_t msClearDefaultPart;
};

#endif // PTGWORLD_H
