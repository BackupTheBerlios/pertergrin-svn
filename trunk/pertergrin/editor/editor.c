/* editor.c - Main function and misc editor functions
 *
 * It is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License,  or (at your option) any later
 * version. You should have received a copy of the GNU General Public License
 * along with this program. If not contact one the AUTHORS.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "editor.h"
#include "interface.h"
#include "support.h"
#include "files.h"
#include "gui.h"

#define DELLOG 1

void CloseAll(void)
{
    FreeGUI();
}

int
main (int argc, char *argv[])
{
    GtkWidget *mainwindow;
    char picname[]=PIXBUFFILE; // Name of map pieces picture files, in DB ?

#ifdef DELLOG
    {
        FILE *fh;

        fh=fopen(getLogfile(),"w");
        fclose(fh);
    }
#endif

#ifdef ENABLE_NLS
    bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
    textdomain (PACKAGE);
#endif

    gtk_set_locale ();
    gtk_init (&argc, &argv);

    add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps");
    add_pixmap_directory (PACKAGE_SOURCE_DIR "/editor/pixmaps");

#if DEBUGLEV > 4
    errormsg(MAPMSG,"\nmain: Trying to load MapPieces Pixbuf...");
#endif
    if (!MapPicLoad(picname))
    {
        printf("Could not load MapPieces Bitmap.\n");
        CloseAll();
        exit(20);
    }

    if (!(mainwindow = InitPTGEditorWindow()))
    {
	printf("Did not get a Window pointer. Continuing anyway..\n");
	//CloseAll();
	//exit(20);
    }
    else EO_Window=mainwindow;

#if DEBUGLEV > 4
    errormsg(MAPMSG,".Done\nMain Loop\n");
#endif

    /* Main Loop */

    gtk_main ();

#if DEBUGLEV > 4
    errormsg(MAPMSG,"Exiting..");
#endif
    CloseAll();
#if DEBUGLEV > 4
    errormsg(MAPMSG,".Done\n");
#endif

  return 0;
}
