/* mainwindow.h - Pertergrin Editor class declaration of main window
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

#ifndef PGTMAINWINDOW_H
#define PGTMAINWINDOW_H

#define GUI_DIRECTORY "gui"

// includes
#include <libglademm.h>
#include <libglademm/variablesmap.h>
#include <gtkmm.h>

#include "worlddialog.h"

// defines
typedef enum
{
  MN_FileNew = 0,
  MN_FileOpen,
  MN_FileSave,
  MN_FileSaveAs,
  MN_FilePrintWorld,
  MN_FileQuit,
  MN_EditCut,
  MN_EditCopy,
  MN_EditPaste,
  MN_EditClear,
  MN_EditSelectAll,
  MN_EditMove,
  MN_EditRefresh,
  MN_EditUndo,
  MN_EditRedo,
  MN_ViewWorldEditor,
  MN_ViewTownEditor,
  MN_ViewDungeonEditor,
  MN_ViewCurrentMapPart,
  MN_ComponentsLandscapePart,
  MN_ComponentsTownPart,
  MN_ComponentsDungeonPart,
  MN_ComponentsCGroup,
  MN_ComponentsComponent,
  MN_ComponentsActionGroup,
  MN_ComponentsAction,
  MN_ComponentsAbilityGroup,
  MN_ComponentsAbility,
  MN_SettingsPreferences,
  MN_SettingsGameOptions,
  MN_SettingsPluginOptions,
  MN_SettingsSave,
  MN_SettingsLoad,
  MN_HelpManual,
  MN_HelpOnlineHelp,
  MN_HelpAbout,
  MN_MenuAnz
} menu_t;

// class declaration

class PTGMainWindow : public Gtk::Window
{

public:
  PTGMainWindow();
  virtual ~PTGMainWindow();

  // Slots for menus
  void on_file_new();
  void on_quit();

protected:
  // Signal handlers
  //virtual void on_button_clicked
  
  Glib::RefPtr<Gnome::Glade::Xml> moRefGlade;
  Gnome::Glade::VariablesMap* mopVariablesMap;

  // Menus
  Gtk::MenuItem *mopMenuItem[MN_MenuAnz];
  Glib::ustring moMenuName[MN_MenuAnz];

  // Dialogs
  PTGWorldDialog *mopWorldDialog;

  // Variables
  //Glib::ustring moStrEntry;
  //bool mbCheckBox;
};

#endif // PGTMAINWINDOW_H
