/* app.cc - Pertergrin Editor class definition of the application management
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

#include "app.h"

// Empty default constructor
PtgApp::PtgApp() : mpoWorld( 0 )
{
}

// Creates a new world
// Input parameters:
// sWorldData  -  Definition of the world
void PtgApp::createWorld(pworld_t sWorldData)
{
  // Have we already created a world ?
  if( mpoWorld.get() != 0 )
    mpoWorld.reset();
  
  auto_ptr<PtgWorld> poWorld(new PtgWorld(sWorldData.iXSize, 
                                          sWorldData.iYSize, 
                                          sWorldData.oName,
                                          sWorldData.iTileWidth, 
                                          sWorldData.iTileHeight)
	                        );

  // Transfer Pointer
  mpoWorld = poWorld;
  mpoWorld->setDescription(sWorldData.oDescription);
  //mpoWorld->setWorldType(WT_Pertergrin);
  //mpoWorld->setRegionSize(sWorldData.iRegionXSize, sWorldData.iRegionYSize);
  //mpoWorld->setStartDate(sWorldData.oStartDate);
}

// Gets the world
// Return parameter:
// Instance of the world (const, so it cannot be destroyed accidently)
const auto_ptr<PtgWorld> &PtgApp::getWorld()
{
  return mpoWorld;
}
  
// Sets/Changes the data of the world
// Input parameters:
// sWorldData  -  Definition of the world
void PtgApp::setWorldData(pworld_t sWorldData)
{
  mpoWorld->setDescription(sWorldData.oDescription);
  mpoWorld->setSize(sWorldData.iXSize, sWorldData.iYSize);
  mpoWorld->setName(sWorldData.oName);
  //mpoWorld->setWorldType(WT_Pertergrin);
  //mpoWorld->setRegionSize(sWorldData.iRegionXSize, sWorldData.iRegionYSize);
  //mpoWorld->setStartDate(sWorldData.oStartDate);
}
