/* worlddialog.cc - Pertergrin Editor class definition of world dialog
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
#include "worlddialog.h"

// defines

// This makes the whole life a lot easier
#define Button_Connect( x, y ) \
        ( x->signal_clicked().connect( SigC::slot(*this, &PTGWorldDialog::y) ) )

// member methods

// Empty default constructor
PTGWorldDialog::PTGWorldDialog(BaseObjectType* pObject, 
                               const Glib::RefPtr<Gnome::Glade::Xml>& oRefGlade)
  : moRefGlade( oRefGlade)
{
  // Sets the border width of the window.
  set_border_width(2);

  // Set new parent widget to this window
  //moRefGlade->reparent_widget("EditWorldTable", *this);

  mopVariablesMap = new Gnome::Glade::VariablesMap(moRefGlade);
  // Connect values of widgets to members
  //mopVariablesMap->connect_widget("entry", mStrEntry);
  //mopVariablesMap->connect_widget("checkbox", mbCheckBox);

  // Connect complex widgets to the dialog read method
  //Object_Connect( "complex_object", on_complex_object );

  // Slots for Ok/Cancel
  Gtk::Button* pButton = 0;
  moRefGlade->get_widget("WorldDialogOkButton",pButton);
  if( pButton )
    Button_Connect( pButton, on_dialog_ok );
  moRefGlade->get_widget("WorldDialogCancelButton",pButton);
  if( pButton )
    Button_Connect( pButton, on_dialog_cancel );

  // ToDo: Catch close connect event!? Treat it as "Ok"

  // Finally show our window
  show_all_children();
  // Maximize window
  //maximize();
}

// Destructor
PTGWorldDialog::~PTGWorldDialog()
{
}

void PTGWorldDialog::getOptions()
{
}

void PTGWorldDialog::setOptions()
{
}

void PTGWorldDialog::on_dialog_ok()
{
  //response(1);
  fprintf(stderr,"Ok\n");
  hide();
}

void PTGWorldDialog::on_dialog_cancel()
{
  //response(0);
  fprintf(stderr,"Cancel\n");
  hide();
}
