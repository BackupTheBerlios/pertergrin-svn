/* Header file for the using interface */

#include <glib.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkfilesel.h>
#include <gtk/gtkframe.h>
#include <gdk/gdktypes.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glade/glade.h>
#include <stdio.h>

#include "mapeditwidget/mapeditclass.h"

/*
**  Object IDs.
*/
enum
{
    EO_MASTER, EO_MEINFO, EO_B, EO_REGX, 
    EO_EDIT, EO_REGY, EO_MXGAD1,

    EO_NUMGADS
};

#define PIXBUFFILE "ptg_landscape.png" // Needs to be changed!! -> DB ?

// The (world) map itself
GtkMapEditClass *WorldMapEditClass;
extern GtkWidget *EO_Window;

// Prototypes
void FreeGUI(void);
BOOL MapPicLoad(char *pngfname);
GtkWidget *mapedit_create_new(gchar *widget_name, gchar *string1, 
                              gchar *string2, gint int1, gint int2);
GtkWidget *InitPTGEditorWindow( void );
