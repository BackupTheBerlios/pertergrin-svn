/* ptgtpes.h - Pertergrin Editor definition of all important structures
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

/* Header file for attributes and general objects in Petergrin       */
/* Attention: The Pertergrin types make use of several glib types    */
/*            This way memory problems are largely reduced!          */

#ifndef PTG_TYPES_H
#define PTG_TYPES_H

#include <glibmm.h>
#include <vector>
#include <string>
using namespace std;

// defines
// Do never ever use #defines for configuration things
// As there is no configuration class I don't use a method
//    to find which version of Pertergrin is used.
#define PTG_PROTOTYPE 1

/* data types: 0 = boolean
 *             1 = integer
 *             2 = double
 *	       3 = text
 */
typedef enum
{
  P_BOOLEAN = 0,
  P_INTEGER,
  P_FLOAT,
  P_TEXT
} ptype_t;

// A normal ability (with a well defined data type)
typedef struct ptg_ability
{
  Glib::ustring   oName;           /* Name of the ability          */
  Glib::ustring   oDescription;    /* Description of the ability   */
  ptype_t         eType;           /* data type of the ability     */
  Glib::ustring   oFormat;         /* How the ability is formatted */
  union 
  {
    bool          bValue  ;        /* boolean value                  */
    int           iValue;          /* integer value                  */
    float         fValue;          /* floating point value           */
  } u;
  Glib::ustring     oValue;        /* text value                     */
} pability_t;

// A normal action (may contain a function handler ?)
typedef struct ptg_action
{
  Glib::ustring      oName;           /* Name of the action        */
  Glib::ustring      oDescription;    /* Description of the action */
  vector<pability_t> vecoAbilities;   /* Abilities of the action   */
  //HDL              handler;         /* Handler of the action     */
} pcaction_t;

// A simple object (only normal abilities)
typedef struct ptg_simpleobj
{
  Glib::ustring      oName;          /* Name of the simple object        */
  Glib::ustring      oDescription;   /* Description of the simple object */
  vector<pability_t> vecoAbilities;  /* Abilities of the simple object   */
} psobj_t;

// A graphic tile to be displayed on the map
typedef struct ptg_gtile
{
  Glib::ustring   oName;          /* Name of the graphic tile        */
  int             iXPos;          /* X-Position inside tiles graphic */
  int             iYPos;          /* Y-Position inside tiles graphic */
  int          iTileset;          /* Tileset to be used              */
} pgtile_t;

// A landscape part (part of the region)
typedef struct ptg_lpart
{
  Glib::ustring      oName;              /* Name of the graphic tile             */
  Glib::ustring      oComponentGroup;    /* component group this part belongs to */
  vector<pability_t> vecoAbilities;      /* Abilities of the part (component)    */
  pgtile_t           oGraphicTile;       /* graphic tile used for this part      */
  vector<int>        veciExits;
} plpart_t;

// The world and the world map
typedef struct ptg_world
{
  Glib::ustring              oName;        /* Name of the world                   */
  Glib::ustring              oDescription; /* Description of the world            */
  Glib::ustring              oWorldType;   /* World type of the world             */
  int                        iXSize;       /* Number of Columns in the world map  */
  int                        iYSize;       /* Number of Rows in the world map     */
  vector< vector<plpart_t> > vecoWorldMap; /* Consists of rows of landscape parts */
  Glib::ustring              oStartDate;   /* Start date (cannot be a date type!) */
  int                        iRegionX;     /* Selected region column              */
  int                        iRegionY;     /* Selected region row                 */
  int                        iRegionXSize; /* Number of Columns in a region       */
  int                        iRegionYSize; /* Number of Rows in a region          */
  int                        iWidth;       /* width of one graphic tile           */
  int                        iHeight;      /* height of one graphic tile          */
} pworld_t;

#endif // PTG_TYPES_H
