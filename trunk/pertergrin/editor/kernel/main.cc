/* main.cc - Pertergrin Editor main function which creates the main window
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

#include <gtkmm/main.h>
#include "mainwindow.h"

int main(int argc, char*argv[])

{
  Gtk::Main kit(argc,argv);
  PTGMainWindow oMainWin;

  kit.run(oMainWin);
  return (0);
}
