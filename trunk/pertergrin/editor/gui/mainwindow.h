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
#include "mapdrawarea.h"
#include "kernel/app.h"

// defines
class PTGMainWindow;

// Should be part of the configuration class
#define PTGTileMapFile "pixmaps/ptg_landscape.png"

// All menus of the main window
typedef enum
{
  MI_FileNew = 0,
  MI_FileChange,
  MI_FileOpen,
  MI_FileSave,
  MI_FileSaveAs,
  MI_FilePrintWorld,
  MI_FileQuit,
  MI_EditCut,
  MI_EditCopy,
  MI_EditPaste,
  MI_EditClear,
  MI_EditSelectAll,
  MI_EditMove,
  MI_EditRefresh,
  MI_EditUndo,
  MI_EditRedo,
  MI_ViewWorldEditor,
  MI_ViewTownEditor,
  MI_ViewDungeonEditor,
  MI_ViewCurrentMapPart,
  MI_ComponentsLandscapePart,
  MI_ComponentsTownPart,
  MI_ComponentsDungeonPart,
  MI_ComponentsCGroup,
  MI_ComponentsComponent,
  MI_ComponentsActionGroup,
  MI_ComponentsAction,
  MI_ComponentsAbilityGroup,
  MI_ComponentsAbility,
  MI_SettingsPreferences,
  MI_SettingsGameOptions,
  MI_SettingsPluginOptions,
  MI_SettingsSave,
  MI_SettingsLoad,
  MI_HelpManual,
  MI_HelpOnlineHelp,
  MI_HelpAbout,
  MI_MenuAnz
} menuitem_t;

// Menu item struct for defining menus
typedef struct
{
  Glib::ustring oName;
  Gtk::MenuItem *opGItem;
  void          (PTGMainWindow::*vpMethod)();
} ptgmenuitem_t;

// class declaration

class PTGMainWindow : public Gtk::Window
{

public:
  PTGMainWindow(BaseObjectType* cobject, 
                const Glib::RefPtr<Gnome::Glade::Xml>& opRefGlade);
  virtual ~PTGMainWindow();

  // Slots for menus
  void on_file_new();
  void on_file_change();
  void on_quit();

protected:
  // Signal handlers
  //virtual void on_button_clicked
  
  Glib::RefPtr<Gnome::Glade::Xml> mopRefGlade;
  Gnome::Glade::VariablesMap* mopVariablesMap;

  // Menus
  ptgmenuitem_t moMItem[MI_MenuAnz];

  // Dialogs
  PTGWorldDialog *mopWorldDialog;

  // Pixmap
  Glib::RefPtr<Gdk::Pixbuf> mopTilesMap;
  Gtk::Viewport *mopMap;

  // Variables
  //Glib::ustring moStrEntry;
  //bool mbCheckBox;

  // Methods
  bool LoadTilesMap();

private:
  PtgApp moApp;
  cwtmap_t moMap;
};

#endif // PGTMAINWINDOW_H
