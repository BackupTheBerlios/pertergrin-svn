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
                               const Glib::RefPtr<Gnome::Glade::Xml>& opRefGlade)
  : Gtk::Dialog(pObject), mopRefGlade( opRefGlade)
{
  // Sets the border width of the window.
  set_border_width(2);

  // Set new parent widget to this window
  //moRefGlade->reparent_widget("EditWorldTable", *this);

  mopVariablesMap = new Gnome::Glade::VariablesMap(mopRefGlade);
  // Connect values of widgets to members
  //mopVariablesMap->connect_widget("entry", mStrEntry);
  //mopVariablesMap->connect_widget("checkbox", mbCheckBox);

  // Get all input widgets from Libglade generated dialog
  mopRefGlade->get_widget("WorldNameEntry",mpoWorldName);
  mopRefGlade->get_widget("WorldStartDateEntry",mpoWorldStartDate);
  mopRefGlade->get_widget("WorldXSizeSpinbutton",mpoWorldXSize);
  mopRefGlade->get_widget("WorldYSizeSpinbutton",mpoWorldYSize);
  mopRefGlade->get_widget("WorldXSizeRegionSpinbutton",mpoWorldXSizeRegion);
  mopRefGlade->get_widget("WorldYSizeRegionSpinbutton",mpoWorldYSizeRegion);
  mopRefGlade->get_widget("WorldTypeCombo",mpoWorldType);
  mopRefGlade->get_widget("WorldDescriptionTextview",mpoWorldDescription);

  // Connect complex widgets to the dialog read method
  //Object_Connect( "complex_object", on_complex_object );

  // Slots for Ok/Cancel
  Gtk::Button* pButton = 0;
  mopRefGlade->get_widget("WorldDialogOkButton",pButton);
  if( pButton )
    Button_Connect( pButton, on_dialog_ok );
  mopRefGlade->get_widget("WorldDialogCancelButton",pButton);
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

// Read all the options from the world dialog
// Output parameter:
// oWorldData  -  World Options read from the world dialog
void PTGWorldDialog::getOptions(pworld_t &oWorldData)
{
  if( mpoWorldName )
    oWorldData.oName = mpoWorldName->get_text();
  if( mpoWorldDescription )
    oWorldData.oDescription = mpoWorldDescription->get_buffer()->get_text();
  if( mpoWorldType )
    oWorldData.oWorldType = mpoWorldType->get_entry()->get_text();
  if( mpoWorldStartDate )
    oWorldData.oStartDate = mpoWorldStartDate->get_text();
  if( mpoWorldXSize )
	oWorldData.iXSize = mpoWorldXSize->get_value_as_int();
  if( mpoWorldYSize )
	oWorldData.iYSize = mpoWorldYSize->get_value_as_int();
  if( mpoWorldXSizeRegion )
	oWorldData.iRegionXSize = mpoWorldXSizeRegion->get_value_as_int();
  if( mpoWorldYSizeRegion )
	oWorldData.iRegionYSize = mpoWorldYSizeRegion->get_value_as_int();
}

// Set all options to the world dialog
// Input parameters:
// oWorldData  -  World options to be shown in the world dialog
void PTGWorldDialog::setOptions(pworld_t oWorldData)
{
  if( mpoWorldName )
    mpoWorldName->set_text( oWorldData.oName );
  if( mpoWorldDescription )
    mpoWorldDescription->get_buffer()->set_text( oWorldData.oDescription );
  if( mpoWorldType )
    mpoWorldType->get_entry()->set_text( oWorldData.oWorldType );
  if( mpoWorldStartDate )
    mpoWorldStartDate->set_text( oWorldData.oStartDate );
  if( mpoWorldXSize )
	mpoWorldXSize->set_value( oWorldData.iXSize );
  if( mpoWorldYSize )
	mpoWorldYSize->set_value( oWorldData.iYSize );
  if( mpoWorldXSizeRegion )
	mpoWorldXSizeRegion->set_value( oWorldData.iRegionXSize );
  if( mpoWorldYSizeRegion )
	mpoWorldYSizeRegion->set_value( oWorldData.iRegionYSize );
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
