/* mainwindow.h - Pertergrin Editor class declaration of main window
 *
 * Copyright (C) 2001-2002 Pertergrin development team
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

#ifndef PGTMAINWINDOW_H
#define PGTMAINWINDOW_H

// includes
#include <libglademm/variablesmap.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>

// defines

// class declaration

class PTGMainWindow
{

public:
  PTGMainWindow();
  virtual ~PTGMainWindow();

protected:
  // Signal handlers
  //virtual void on_button_clicked
  
  Glib::RefPtr<Gnome::Glade::Xml> moRefGlade;
  Gnome::Glade::VariablesMap* mopVariablesMap;

  // Variables
  //Glib::ustring moStrEntry;
  //bool mbCheckBox;
};

#endif // PGTMAINWINDOW_H
