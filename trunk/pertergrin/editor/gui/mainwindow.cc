/* mainwindow.cc - Pertergrin Editor class definition of main window
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

// includes
#include "mainwindow.h"

// defines

// member methods

// Empty default constructor
PTGMainWindow::PTGMainWindow()
{
  // Sets the border width of the window.
  set_border_width(10);

  // Get Glade UI
  moRefGlade = Gnome::Glade::Xml::create("example.glade");
  // Set new parent widget to this window
  moRefGlade->reparent_widget("vbox", *this);

  mopVariablesMap = new Gnome::Glade::VariablesMap(m_refGlade);
  // Connect values of widgets to members
  //mopVariablesMap->connect_widget("entry", mStrEntry);
  //mopVariablesMap->connect_widget("checkbox", mbCheckBox);

  //Connect signal handler
  //Gtk::Button* pButton = 0;
  //mRefGlade->get_widget("button", pButton);
  //if(pButton)
  //  pButton->signal_clicked().connect( SigC::slot(*this, &ExampleWindow::on_button_clicked) );

  // Finally show our window
  show_all_children();
}

// Destructor
PTGMainWindow::PTGMainWindow()
{
}

/*
void PTGMainWindow::on_button_clicked()
{
  if(mopVariablesMap)
    mopVariablesMap->transfer_widgets_to_variables();

}
*/
