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
#include "kernel/ptgtypes.h"

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
  moMItem[MI_FileChange].oName = "change_world";
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
  moMItem[MI_FileNew].vpMethod = &PTGMainWindow::on_file_new;
  moMItem[MI_FileChange].vpMethod = &PTGMainWindow::on_file_change;
  moMItem[MI_FileQuit].vpMethod = &PTGMainWindow::on_quit;

  // If all menu items can be accessed the next two blocks are in a for loop
  // Get menu items from glade file
  mopRefGlade->get_widget(moMItem[MI_FileNew].oName,moMItem[MI_FileNew].opGItem);
  mopRefGlade->get_widget(moMItem[MI_FileChange].oName,moMItem[MI_FileChange].opGItem);
  mopRefGlade->get_widget(moMItem[MI_FileQuit].oName,moMItem[MI_FileQuit].opGItem);

  moMItem[MI_FileChange].opGItem->set_sensitive( false );

  // Connect all menu items to slots (see macro above)
  if ( moMItem[MI_FileQuit].opGItem )
    Menu_Connect( moMItem[MI_FileQuit].opGItem, moMItem[MI_FileQuit].vpMethod );

  if ( moMItem[MI_FileNew].opGItem )
    Menu_Connect( moMItem[MI_FileNew].opGItem, moMItem[MI_FileNew].vpMethod );

  if ( moMItem[MI_FileChange].opGItem )
    Menu_Connect( moMItem[MI_FileChange].opGItem, moMItem[MI_FileChange].vpMethod );
  
  // Create application kernel
  
  // Finally show our window
  show_all_children();
  // Maximize window
  maximize();
}

// Destructor
PTGMainWindow::~PTGMainWindow()
{
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
    pworld_t sWorldData;

    // Already created the world ?
    if( moApp.getWorld().get() != 0 )
    {
      // Display warning
      Gtk::MessageDialog oDestroyWarn("Really destroy the current world?", 
                                      Gtk::MESSAGE_WARNING,
                                      Gtk::BUTTONS_YES_NO
		                             );
      if( oDestroyWarn.run() != Gtk::RESPONSE_YES )
        return;
    }
    iRes = mopWorldDialog->run();
    mopWorldDialog->hide();
    // If Ok was pressed, fetch the world options
    if( iRes == Gtk::RESPONSE_ACCEPT || iRes == Gtk::RESPONSE_OK )
    {
      // Read world options from dialog
      mopWorldDialog->getOptions(sWorldData);
      moApp.createWorld(sWorldData);
      // Load Tiles
      LoadTilesMap();
      // Show change menu item
      moMItem[MI_FileChange].opGItem->set_sensitive( true );
      // Show argh does not work so easily....
    }
	// Done reading, mopWorldDialog will be deleted
	mopWorldDialog = 0;
  }
}

void PTGMainWindow::on_file_change()
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
    pworld_t sWorldData;

    // Already created the world ?
    if( moApp.getWorld().get() == 0 )
      return; // Nothing to do
    // Read current world data
    sWorldData = moApp.getWorld()->getWorldData();
    // Show options of world data in dialog
    mopWorldDialog->setOptions( sWorldData );
    // Show world options
    iRes = mopWorldDialog->run();
    mopWorldDialog->hide();
    // If Ok was pressed, fetch the world options
    if( iRes == Gtk::RESPONSE_ACCEPT || iRes == Gtk::RESPONSE_OK )
    {
      // Read world options from dialog
      mopWorldDialog->getOptions(sWorldData);
      moApp.setWorldData( sWorldData );
    }
	// Done reading, mopWorldDialog will be deleted
	mopWorldDialog = 0;
  }
}

bool PTGMainWindow::LoadTilesMap()
{
  mopTilesMap = Gdk::Pixbuf::create_from_file( PTGTileMapFile );
  if( mopTilesMap != 0 )
    return true;
  return false;
}
