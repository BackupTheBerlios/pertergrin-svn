/* app.h - Pertergrin Editor class declaration of the application management
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

#ifndef PTGAPP_H
#define PTGAPP_H

#include "ptgtypes.h"
#include "world.h"
#include <memory>

class PtgApp
{
public:
  PtgApp();

  // World methods
  void createWorld(pworld_t sWorldData);
  void setWorldData(pworld_t sWorldData);
  // Read only
  const auto_ptr<PtgWorld> &getWorld();

private:
  auto_ptr<PtgWorld> mpoWorld;

};

#endif // PTGAPP_H
