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
        ( x->signal_activate().connect( SigC::slot(*this, &PTGMainWindow::y) ) )


// member methods

// Empty default constructor
PTGMainWindow::PTGMainWindow()
{
  // Initialisation of member variables
  mopWorldDialog = 0;

  // Sets the border width of the window.
  set_border_width(2);

  // Get Glade UI
  moRefGlade = Gnome::Glade::Xml::create(Glib::ustring(GUI_DIRECTORY) +
                                         Glib::ustring("/mainwindow.glade"));
  // Set new parent widget to this window
  moRefGlade->reparent_widget("mainvbox", *this);

  mopVariablesMap = new Gnome::Glade::VariablesMap(moRefGlade);
  // Connect values of widgets to members
  //mopVariablesMap->connect_widget("entry", mStrEntry);
  //mopVariablesMap->connect_widget("checkbox", mbCheckBox);

  // Define menu names (is there any way to get them from the glade file ?!?)
  moMenuName[MN_FileNew] = "new_world";
  moMenuName[MN_FileOpen] = "open_world";
  moMenuName[MN_FileSave] = "save_world";
  moMenuName[MN_FileSaveAs] = "save_world_as";
  moMenuName[MN_FilePrintWorld] = "print_world";
  moMenuName[MN_FileQuit] = "quit";
  moMenuName[MN_EditCut] = "cut";
  moMenuName[MN_EditCopy] = "copy";
  moMenuName[MN_EditPaste] = "paste";
  moMenuName[MN_EditClear] = "clear";
  moMenuName[MN_EditSelectAll] = "select_all";
  moMenuName[MN_EditMove] = "move";
  moMenuName[MN_EditRefresh] = "refresh";
  moMenuName[MN_EditUndo] = "undo";
  moMenuName[MN_EditRedo] = "redo";
  moMenuName[MN_ViewWorldEditor] = "world_editor";
  moMenuName[MN_ViewTownEditor] = "town_editor";
  moMenuName[MN_ViewDungeonEditor] = "dungeon_editor";
  moMenuName[MN_ViewCurrentMapPart] = "current_map_part";
  moMenuName[MN_ComponentsLandscapePart] = "landscape_part";
  moMenuName[MN_ComponentsTownPart] = "town_part";
  moMenuName[MN_ComponentsDungeonPart] = "dungeon_part";
  moMenuName[MN_ComponentsCGroup] = "component_group";
  moMenuName[MN_ComponentsComponent] = "component";
  moMenuName[MN_ComponentsActionGroup] = "action_group";
  moMenuName[MN_ComponentsAction] = "action";
  moMenuName[MN_ComponentsAbilityGroup] = "abilit_group";
  moMenuName[MN_ComponentsAbility] = "ability";
  moMenuName[MN_SettingsPreferences] = "preferences";
  moMenuName[MN_SettingsGameOptions] = "game_options";
  moMenuName[MN_SettingsPluginOptions] = "plugin_options";
  moMenuName[MN_SettingsSave] = "settings_save";
  moMenuName[MN_SettingsLoad] = "settings_load";
  moMenuName[MN_HelpManual] = "manual";
  moMenuName[MN_HelpOnlineHelp] = "online_help";
  moMenuName[MN_HelpAbout] = "about";

  // Get menu items from glade file
  moRefGlade->get_widget(moMenuName[MN_FileNew],mopMenuItem[MN_FileNew]);
  moRefGlade->get_widget(moMenuName[MN_FileQuit],mopMenuItem[MN_FileQuit]);

  // Connect all menu items to slots (see macro above)
  if ( mopMenuItem[MN_FileQuit] )
    Menu_Connect( mopMenuItem[MN_FileQuit], on_quit );

  if ( mopMenuItem[MN_FileNew] )
    Menu_Connect( mopMenuItem[MN_FileNew], on_file_new);

  //Connect signal handler
  //Gtk::Button* pButton = 0;
  //moRefGlade->get_widget("button", pButton);
  //if(pButton)
  //  pButton->signal_clicked().connect( SigC::slot(*this, &ExampleWindow::on_button_clicked) );

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
  Glib::RefPtr<Gnome::Glade::Xml> oRefGlade;
  oRefGlade = Gnome::Glade::Xml::create(Glib::ustring(GUI_DIRECTORY) +
                                        Glib::ustring("/worlddialog.glade"));
  // Open world dialog
  if( !mopWorldDialog )
  {
    oRefGlade->get_widget_derived("WorldDialog", mopWorldDialog);
    //mopWorldDialog = new PTGWorldDialog();
  }
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

/*
void PTGMainWindow::on_button_clicked()
{
  if(mopVariablesMap)
    mopVariablesMap->transfer_widgets_to_variables();

}
*/
