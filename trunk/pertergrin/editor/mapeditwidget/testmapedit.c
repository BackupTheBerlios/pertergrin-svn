/*
**  testmapedit.c (C) 2000/2001 Reinhard Katzmann. All rights reserved.
**
**  A program to test the mapeditclass widget. Like the mapeditclass
**  itself it is distributed under the GPL License. See the mapeditclass
**  widget source or documentation for more informations.
*/

/*
#define USE_DMALLOC 1

#define MALLOC(size){ (malloc(size)) }
#define ALLOC(type,size) (malloc(sizeof(type)*size))
#define REALLOC(mem,type,size) (realloc(mem, sizeof(type)*size))
#define CALLOC(type,size) (calloc(sizeof(type), size))
#define FREE(mem) (free(mem))
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <memory.h>
#include <glib.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkfilesel.h>
#include <gtk/gtkframe.h>
#include <gdk/gdktypes.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glade/glade.h>

#include <widgets/mapeditclass.h>

#define SELXSIZE 4
#define SELYSIZE 4

#define DEBUGLEV 5       /* Must be the same as in mapeditclass.c!!! */

#define DELLOG 1

/*
**  Object IDs.
*/
enum
{
    WO_MASTER, WO_MEINFO, WO_B, WO_GRID,
    WO_FRAME, WO_REGX, WO_EDIT, WO_REGY,
    WO_MXGAD1, WO_MPGRP1, WO_MPGRP2, 
    WO_MPGRP3, WO_LOAD, WO_SAVE,
    WO_QUIT,

    WO_NUMGADS
};

// Name of Glade GUI file name, Graphic File name
#define TESTMAPEDITFILE "testmapedit.glade"
#define PIXBUFFILE "breakout.png"

// Mapedit custom ids
enum { MPGR1, MPGR2, MPGR3, MPEDIT };

GtkMapEditClass *MapEditClass;
GtkWidget *TestMEdWindowObjs[WO_NUMGADS], *WO_Window;;
GtkFileSelection *filereq;
GdkPixbuf *mMapPieces = NULL;

guchar *WO_MXGAD1Labels[] =
{
    "MPG1",
    "MPG2",
    "MPG3",
    NULL
};

void InitSelectGroup (struct MCMap *SelectGroup, gint start);
char *getLogfile();

char *getLogfile()
{
    return "testmapedit.log";
}

void CloseAll(void)
{
    struct MCMap *SelectGroup = NULL;
    GtkArg mapedarg;
    guchar i;

    errormsg(MAPDEBUG1,"CloseAll: Entered");

    if (mMapPieces) gdk_pixbuf_unref(mMapPieces);
    //if (filereq) DisposeObject( filereq);
    for (i=0; i<3; i++)
      if (!TestMEdWindowObjs[WO_MPGRP1+i])
      {
        mapedarg.type = MAPEDIT_Map;
        gtk_object_getv(GTK_OBJECT(TestMEdWindowObjs[WO_MPGRP1+i]), 1, 
			&mapedarg);
        SelectGroup = (struct MCMap *) GTK_VALUE_POINTER(mapedarg);
#if DEBUGLEV > 2
        errormsg(MAPDEBUG3,"CloseAll: Map=%x, pt=%x, SelectGroup=%x",
		 (struct MCMap *)TestMEdWindowObjs[WO_MPGRP1+i], 
		 mapedarg.d.pointer_data, SelectGroup);
#endif
        if (SelectGroup)
        {
            do
            {
                struct MCMap *tmap=NULL;
#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"CloseAll: SelectGroup=%x, "
			 "SelectGroup->mm_Columns=%x", SelectGroup, 
			 SelectGroup->mm_Columns);
#endif
                if (SelectGroup->mm_Columns)
                {
                    int i;
                    for (i=0; i< SELYSIZE;i++) // All columns have to be freed
                    {
#if DEBUGLEV > 2
		      errormsg(MAPDEBUG3,"CloseAll: SelectGroup->mm_Columns"
			       "[%d]=%x", i, SelectGroup->mm_Columns[i]);
#endif
                        if (SelectGroup->mm_Columns[i])
                            g_free((void *)SelectGroup->mm_Columns[i]);
                        SelectGroup->mm_Columns[i]=0;
                    }
                    g_free(SelectGroup->mm_Columns);
                    SelectGroup->mm_Columns=NULL;
                }
#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"CloseAll: SelectGroup->mm_NextLayer=%x",
			 SelectGroup->mm_NextLayer);
#endif
                tmap = SelectGroup->mm_NextLayer; // Get pointer to next layer
                SelectGroup->mm_NextLayer = NULL; // Do not reference it again!
                g_free(SelectGroup);        // Free this layer
                SelectGroup=tmap;     // Set new current layer
            } while (SelectGroup);
        }
    }

    errormsg(MAPDEBUG1,"CloseAll: Finished succesfully");
}

/*
**      Put up a simple dialog.
**/
gulong Req( char *text )
{
  return 0L;
}

/*
** Load the MapPieces Pixbuf 
*/
BOOL MapPicLoad(void)
{
    errormsg(MAPDEBUG1,"MapPicLoad: Entered");

    gdk_rgb_init (); // Required for rendering! Really put here ?
    mMapPieces = gdk_pixbuf_new_from_file ("breakout.png");
    g_assert (mMapPieces != NULL);

    errormsg(MAPDEBUG1,"MapPicLoad: Finished succesfully");
    return TRUE;
}

/*
** GUI Handler: Called when "custom" mapedit widget should be created
*/
GtkWidget *mapedit_create_new(gchar *widget_name, gchar *string1, 
			      gchar *string2, gint int1, gint int2)
{
    struct MCMap *mapeditmap;
    GtkWidget *mapwidget=NULL;
    GtkArg mapedarg[11], *args = mapedarg;

    struct MapPiece mDefault = {50, 0, { 0, 0, 0 }, { 0, 0, 0 }, 0, NULL};
    errormsg(MAPDEBUG1,"mapedit_create_new: Entered");

    if (int1!=MPEDIT)
    {
	mapeditmap = g_malloc0(sizeof(struct MCMap));
	if (!mapeditmap)
	{
	    printf("Could not allocate memory for mapeditmap\n");
	    CloseAll();
	    exit(20);
	}
    }
#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"mapedit_create_new: widget_name=%s=%x, int1=%d, "
	     "mapeditmap=%x ",widget_name, widget_name, int1, mapeditmap);
#endif

    mapedarg[0].type = MAPEDIT_MapPieces;
    GTK_VALUE_POINTER(mapedarg[0]) = mMapPieces;
    mapedarg[1].type = MAPEDIT_PWidth;
    GTK_VALUE_UINT(mapedarg[1]) = 37;
    mapedarg[2].type = MAPEDIT_PLength;
    GTK_VALUE_UINT(mapedarg[2]) = 13;
    mapedarg[3].type = MAPEDIT_Default;
    GTK_VALUE_POINTER(mapedarg[3]) = &mDefault;
    mapedarg[4].type = MAPEDIT_MapWidth;
    GTK_VALUE_UINT(mapedarg[4]) = SELXSIZE;
    mapedarg[5].type = MAPEDIT_MapLength;
    GTK_VALUE_UINT(mapedarg[5]) = SELYSIZE;
    mapedarg[6].type = MAPEDIT_MapLayer;
    GTK_VALUE_UCHAR(mapedarg[6]) = 1;
    mapedarg[7].type = MAPEDIT_Map;
    GTK_VALUE_POINTER(mapedarg[7]) = mapeditmap;
    mapedarg[8].type = MAPEDIT_GetPieces;
    GTK_VALUE_BOOL(mapedarg[8]) = TRUE;
    mapedarg[9].type = MAPEDIT_Frame;
    GTK_VALUE_BOOL(mapedarg[9]) = TRUE;
    mapedarg[10].type = MAPEDIT_FrameSpace;
    GTK_VALUE_UCHAR(mapedarg[10]) = 4;

    errormsg(MAPDEBUG3,"mapedit_create_new: args=%x, args[0]=%x, args[1].type=%d",args,args[2].type,MAPEDIT_PWidth);

    switch(int1)
    {
        case MPGR1:
	    InitSelectGroup(mapeditmap,0);
	    TestMEdWindowObjs[WO_MPGRP1]=MapEditClassNew(args, 11);
	    mapwidget=TestMEdWindowObjs[WO_MPGRP1];
	    break;
        case MPGR2:
	    InitSelectGroup(mapeditmap,SELXSIZE*SELYSIZE);
	    TestMEdWindowObjs[WO_MPGRP2]=MapEditClassNew(args, 11);
	    mapwidget=TestMEdWindowObjs[WO_MPGRP2];
	    break;
        case MPGR3:
	    InitSelectGroup(mapeditmap,SELXSIZE*SELYSIZE*2);
	    TestMEdWindowObjs[WO_MPGRP3]=MapEditClassNew(args, 11);
	    mapwidget=TestMEdWindowObjs[WO_MPGRP3];
	    break;
        case MPEDIT:
	    mapedarg[4].type = MAPEDIT_MapWidth;
	    GTK_VALUE_UINT(mapedarg[4]) = 40;
	    mapedarg[5].type = MAPEDIT_MapLength;
	    GTK_VALUE_UINT(mapedarg[5]) = 80;
	    GTK_VALUE_UCHAR(mapedarg[6]) = 2; // 2 Layers!
	    TestMEdWindowObjs[WO_EDIT]=MapEditClassNew(args, 7);
	    mapwidget=TestMEdWindowObjs[WO_EDIT];
	    break;
    }
    errormsg(MAPDEBUG3,"mapedit_create_new: mapwidget=%x",mapwidget);

    errormsg(MAPDEBUG1,"mapedit_create_new: Finished succesfully");
    return mapwidget;
}

void InitSelectGroup (struct MCMap *SelectGroup, gint start)
{
    gulong i,j;

    errormsg(MAPDEBUG1,"InitSelectGroup: Entered");

    SelectGroup->mm_MapSize.x = SELXSIZE;
    SelectGroup->mm_MapSize.y = SELYSIZE;
    SelectGroup->mm_MapSize.l = 2;
    SelectGroup->mm_Copy = FALSE;
    SelectGroup->mm_Size = sizeof(struct MCMap);
    SelectGroup->mm_NextLayer = NULL;
    SelectGroup->mm_Columns = g_malloc0(sizeof(gulong)*SELYSIZE);
    if (!SelectGroup->mm_Columns)
    {
      printf("Could not allocate memory for SelectGroup\n");
      CloseAll();
      exit(20);
    }
#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"InitSelectGroup: SelectGroup->mm_Columns=%x, ",
	     SelectGroup->mm_Columns);
#endif

    for(i=0;i<SELYSIZE;i++)
    {
        SelectGroup->mm_Rows = g_malloc0(sizeof(struct MapPiece)*SELXSIZE);
#if DEBUGLEV > 5
	errormsg(MAPDEBUG3,"InitSelectGroup: i=%d, SelectGroup->mm_Rows=%x",i,
		 SelectGroup->mm_Rows);
#endif
        if (!SelectGroup->mm_Rows)
        {
            printf("Could not allocate memory for SelectGroup\n");
            CloseAll();
            exit(20);
        }
        SelectGroup->mm_Columns[i] = (gulong)SelectGroup->mm_Rows;

        for(j=0;j<SELXSIZE;j++)
        {
#if DEBUGLEV > 5
	    errormsg(MAPDEBUG6,"InitSelectGroup: j=%d",j);
#endif
            SelectGroup->mm_Rows[j].mp_Coordinates.x = j * 37;
            SelectGroup->mm_Rows[j].mp_Coordinates.y = i * 13;
            SelectGroup->mm_Rows[j].mp_Coordinates.l = 1;
            SelectGroup->mm_Rows[j].mp_PixbufNumber = start+i*SELYSIZE+j;
            SelectGroup->mm_Rows[j].mp_Size = sizeof(struct MapPiece);
        }
    }

    errormsg(MAPDEBUG1,"InitSelectGroup: Finished succesfully");
}

/*
** Load and set up GUI with libglade
*/
GtkWidget *InitTestMEdWindow( void )
{
    GtkWidget *win=NULL, *frame=NULL;
    GladeXML *xml=NULL;
    char *rootnode=NULL; // This is probably never needed

    errormsg(MAPDEBUG1,"InitTestMEdWindow: Entered");

    glade_init();
    errormsg(MAPDEBUG4,"InitTestMEdWindow: glade_init done");
    xml = glade_xml_new(TESTMAPEDITFILE, rootnode);

    errormsg(MAPDEBUG4,"InitTestMEdWindow: glade_xml_new done");

    if (!xml) {
        g_warning("something bad happened while creating the interface");
	CloseAll();
	exit(20);
    }

    if (rootnode) // Probably never needed
    {
	GtkWidget *wid = glade_xml_get_widget(xml, rootnode);
	if (!GTK_IS_WINDOW(wid))
	{
	    win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	    errormsg(MAPDEBUG3,"InitTestMEdWindow: win=%x",win);
	    gtk_signal_connect(GTK_OBJECT(win), "destroy",
			       GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
	    gtk_container_set_border_width(GTK_CONTAINER(win), 5);
	    frame = gtk_frame_new(NULL);
	    gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	    gtk_container_add(GTK_CONTAINER(win), frame);
	    gtk_widget_show(frame);
	    gtk_container_add(GTK_CONTAINER(frame), wid);
	    gtk_widget_show(win);
	}
	else win=wid;
    }

    // Connect signals
    errormsg(MAPDEBUG3,"InitTestMEdWindow: win=%x",win);
    glade_xml_signal_autoconnect(xml);
    gtk_object_unref(GTK_OBJECT(xml));
    errormsg(MAPDEBUG3,"InitTestMEdWindow: win=%x now!",win);

    errormsg(MAPDEBUG1,"InitTestMEdWindow: Finished succesfully");
    return win; // Problem: We never get a window pointer ?
}

int main( int argc, char **argv )
{
#ifdef DELLOG
    {
        FILE *fh;

        fh=fopen(getLogfile(),"w");
	fclose(fh);
    }
#endif
    gtk_init(&argc, &argv);

#if DEBUGLEV > 4
    errormsg(MAPMSG,"\nmain: Trying to load MapPieces Pixbuf...");
#endif
    if (!MapPicLoad())
    {
        printf("Could not load MapPieces Bitmap.\n");
        CloseAll();
        exit(20);
    }
#if DEBUGLEV > 4
    errormsg(MAPMSG,".Done\nmain: Trying to initialize window..");
#endif

    if (!(WO_Window=InitTestMEdWindow()))
    {
        printf("Did not get a Window pointer. Continuing anyway..\n");
        //CloseAll();
        //exit(20);
    }
#if DEBUGLEV > 4
    errormsg(MAPMSG,".Done\nMain Loop\n");
#endif

    /* Main Loop */

    gtk_main();

#if DEBUGLEV > 4
    errormsg(MAPMSG,"Exiting..");
#endif
    CloseAll();
#if DEBUGLEV > 4
    errormsg(MAPMSG,".Done\n");
#endif
    exit(0);
}
