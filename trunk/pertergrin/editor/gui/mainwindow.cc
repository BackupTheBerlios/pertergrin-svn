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

// This makes the whole life a lot easier
#define Menu_Connect( x, y ) \
        ( x->signal_activate().connect( SigC::slot(*this, y) ) )


// member methods

// Empty default constructor
PTGMainWindow::PTGMainWindow(BaseObjectType* opObject, 
                 const Glib::RefPtr<Gnome::Glade::Xml>& opRefGlade)
  : Gtk::Window(opObject), mopRefGlade( opRefGlade) 
{
  // Initialisation of member variables
  mopWorldDialog = 0;

  // Sets the border width of the window.
  set_border_width(2);

  mopVariablesMap = new Gnome::Glade::VariablesMap(mopRefGlade);
  // Connect values of widgets to members
  //mopVariablesMap->connect_widget("entry", mStrEntry);
  //mopVariablesMap->connect_widget("checkbox", mbCheckBox);

  // Defining the menu (items, names, methods) to be done in an init method 

  // Define menu names (is there any way to get them from the glade file ?!?)
  moMItem[MI_FileNew].oName = "new_world";
  moMItem[MI_FileOpen].oName = "open_world";
  moMItem[MI_FileSave].oName = "save_world";
  moMItem[MI_FileSaveAs].oName = "save_world_as";
  moMItem[MI_FilePrintWorld].oName = "print_world";
  moMItem[MI_FileQuit].oName = "quit";
  moMItem[MI_EditCut].oName = "cut";
  moMItem[MI_EditCopy].oName = "copy";
  moMItem[MI_EditPaste].oName = "paste";
  moMItem[MI_EditClear].oName = "clear";
  moMItem[MI_EditSelectAll].oName = "select_all";
  moMItem[MI_EditMove].oName = "move";
  moMItem[MI_EditRefresh].oName = "refresh";
  moMItem[MI_EditUndo].oName = "undo";
  moMItem[MI_EditRedo].oName = "redo";
  moMItem[MI_ViewWorldEditor].oName = "world_editor";
  moMItem[MI_ViewTownEditor].oName = "town_editor";
  moMItem[MI_ViewDungeonEditor].oName = "dungeon_editor";
  moMItem[MI_ViewCurrentMapPart].oName = "current_map_part";
  moMItem[MI_ComponentsLandscapePart].oName = "landscape_part";
  moMItem[MI_ComponentsTownPart].oName = "town_part";
  moMItem[MI_ComponentsDungeonPart].oName = "dungeon_part";
  moMItem[MI_ComponentsCGroup].oName = "component_group";
  moMItem[MI_ComponentsComponent].oName = "component";
  moMItem[MI_ComponentsActionGroup].oName = "action_group";
  moMItem[MI_ComponentsAction].oName = "action";
  moMItem[MI_ComponentsAbilityGroup].oName = "abilit_group";
  moMItem[MI_ComponentsAbility].oName = "ability";
  moMItem[MI_SettingsPreferences].oName = "preferences";
  moMItem[MI_SettingsGameOptions].oName = "game_options";
  moMItem[MI_SettingsPluginOptions].oName = "plugin_options";
  moMItem[MI_SettingsSave].oName = "settings_save";
  moMItem[MI_SettingsLoad].oName = "settings_load";
  moMItem[MI_HelpManual].oName = "manual";
  moMItem[MI_HelpOnlineHelp].oName = "online_help";
  moMItem[MI_HelpAbout].oName = "about";

  // Define menu methods (read the handlers from the glade file ?)
  moMItem[MI_FileNew].fpMethod = &PTGMainWindow::on_file_new;
  moMItem[MI_FileQuit].fpMethod = &PTGMainWindow::on_quit;

  // If all menu items can be accessed the next two blocks are in a foor loop
  // Get menu items from glade file
  mopRefGlade->get_widget(moMItem[MI_FileNew].oName,moMItem[MI_FileNew].opGItem);
  mopRefGlade->get_widget(moMItem[MI_FileQuit].oName,moMItem[MI_FileQuit].opGItem);

  // Connect all menu items to slots (see macro above)
  if ( moMItem[MI_FileQuit].opGItem )
    Menu_Connect( moMItem[MI_FileQuit].opGItem, moMItem[MI_FileQuit].fpMethod );

  if ( moMItem[MI_FileNew].opGItem )
    Menu_Connect( moMItem[MI_FileNew].opGItem, moMItem[MI_FileNew].fpMethod );

  // Finally show our window
  show_all_children();
  // Maximize window
  maximize();
}

// Destructor
PTGMainWindow::~PTGMainWindow()
{
  if( mopWorldDialog )
  {
    delete mopWorldDialog;
    mopWorldDialog = 0;
  }
}

// Call this slot to quit the program
void PTGMainWindow::on_quit()
{
  Gtk::Main::quit();
}

// Call this slot to create a new world
void PTGMainWindow::on_file_new()
{
  int iRes = 0;
  Glib::RefPtr<Gnome::Glade::Xml> opRefGlade;
  opRefGlade = Gnome::Glade::Xml::create(Glib::ustring(GUI_DIRECTORY) +
                                         Glib::ustring("/worlddialog.glade"));
  // Open world dialog
  if( !mopWorldDialog )
    opRefGlade->get_widget_derived("WorldDialog", mopWorldDialog);

  if( mopWorldDialog )
  {
    iRes = mopWorldDialog->run();
    fprintf(stderr,"Res: %d\n",iRes);
    mopWorldDialog->hide();
    // If Ok was pressed, fetch the world options
    if( iRes )
      mopWorldDialog->getOptions();
  }
}
