/* worlddialog.h - Pertergrin Editor class declaration of world dialog
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

#ifndef PGTWORLDDIALOG_H
#define PGTWORLDDIALOG_H

#define GUI_DIRECTORY "gui"

// includes
#include <libglademm.h>
#include <libglademm/variablesmap.h>
#include <gtkmm.h>
#include "world.h"

// defines

// class declaration

class PTGWorldDialog : public Gtk::Dialog
{

public:
  PTGWorldDialog(BaseObjectType* cobject, 
                 const Glib::RefPtr<Gnome::Glade::Xml>& opRefGlade);
  virtual ~PTGWorldDialog();

  // Slot which reacts on Ok / Cancel pressed
  void on_dialog_ok();
  void on_dialog_cancel();

  // fetch values being set by the user in the dialog
  void getOptions(pworld_t &oWorldData);
  // set the default values for the dialog
  void setOptions(pworld_t oWorldData);

protected:
  Glib::RefPtr<Gnome::Glade::Xml> mopRefGlade;
  Gnome::Glade::VariablesMap* mopVariablesMap;

  Gtk::Entry *mpoWorldName;
  Gtk::Entry *mpoWorldStartDate;
  Gtk::SpinButton *mpoWorldXSize;
  Gtk::SpinButton *mpoWorldYSize;
  Gtk::SpinButton *mpoWorldXSizeRegion;
  Gtk::SpinButton *mpoWorldYSizeRegion;
  Gtk::Combo *mpoWorldType;
  Gtk::TextView *mpoWorldDescription;  
};

#endif // PGTWORLDDIALOG_H
