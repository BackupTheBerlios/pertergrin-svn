/*
 * GUI for the Editor of Pertergrin
 *
 * It is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License,  or (at your option) any later
 * version. You should have received a copy of the GNU General Public License
 * along with this program. If not contact one the AUTHORS.
 */

#include "gui.h"
#include "editor.h" // CloseAll()

#define DEBUGLEV 5       /* Must be the same as in mapeditclass.c!!! */

// Name of Glade GUI file name, Graphic File name
#define PTGMAPEDITORFILE "mapeditor.glade"

GtkWidget **PTGEditorWindowObjs=NULL, *EO_Window=NULL;
GtkFileSelection *filereq;
GdkPixbuf *mMapPieces = NULL;
struct MapPiece mDefault = {50, 12, { 0, 0, 0 }, { 176, 0, 0 }, 0, NULL};

void InitSelectGroup (struct MCMap *SelectGroup, gint start);

// Number of groups for map pieces
guint ngroup=0;

// Size of the map for the map pieces to select
guint selxsize=3, selysize=10;

// Size of the map pieces itself
guint mpxsize=16, mpysize=16;

// Size of the map
guint mapxsize=600, mapysize=400;

#define MPEDIT 0

void FreeGUI(void)
{
    struct MCMap *SelectGroup = NULL;
    GtkArg mapedarg;
    guchar i;

    errormsg(MAPDEBUG1,"FreeGUI: Entered");

    if (mMapPieces) gdk_pixbuf_unref(mMapPieces);
    //if (filereq) DisposeObject( filereq);
    for (i=0; i<ngroup; i++)
      if (PTGEditorWindowObjs[EO_NUMGADS+i])
      {
	g_return_if_fail (GTK_IS_OBJECT (PTGEditorWindowObjs[EO_NUMGADS+i]));

        mapedarg.type = MAPEDIT_Map;
        gtk_object_getv(GTK_OBJECT(PTGEditorWindowObjs[EO_NUMGADS+i]), 1, 
                        &mapedarg);
        SelectGroup = (struct MCMap *) GTK_VALUE_POINTER(mapedarg);
#if DEBUGLEV > 2
        errormsg(MAPDEBUG3,"CloseAll: Map=%x, pt=%x, SelectGroup=%x",
                 (struct MCMap *)PTGEditorWindowObjs[EO_NUMGADS+i], 
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
                    for (i=0; i< selysize;i++) // All columns have to be freed
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
** Load the MapPieces Pixbuf 
*/
BOOL MapPicLoad(char *pngfname)
{
    errormsg(MAPDEBUG1,"MapPicLoad: Entered");

    gdk_rgb_init (); // Required for rendering! Really put here ?
    mMapPieces = gdk_pixbuf_new_from_file (pngfname);
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
    struct MCMap *mapeditmap=NULL;
    GtkWidget *mapwidget=NULL;
    GtkArg mapedarg[11], *args = mapedarg;

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
    GTK_VALUE_UINT(mapedarg[1]) = mpxsize;
    mapedarg[2].type = MAPEDIT_PLength;
    GTK_VALUE_UINT(mapedarg[2]) = mpysize;
    mapedarg[3].type = MAPEDIT_Default;
    GTK_VALUE_POINTER(mapedarg[3]) = &mDefault;
    mapedarg[4].type = MAPEDIT_MapWidth;
    GTK_VALUE_UINT(mapedarg[4]) = selxsize;
    mapedarg[5].type = MAPEDIT_MapLength;
    GTK_VALUE_UINT(mapedarg[5]) = selysize;
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

    errormsg(MAPDEBUG3,"mapedit_create_new: args=%x, args[0]=%x,"
	     " args[1].type=%d",args,args[2].type,MAPEDIT_PWidth);

    switch(int1)
    {
        case MPEDIT:
            mapedarg[4].type = MAPEDIT_MapWidth;
            GTK_VALUE_UINT(mapedarg[4]) = mapxsize;
            mapedarg[5].type = MAPEDIT_MapLength;
            GTK_VALUE_UINT(mapedarg[5]) = mapysize;
            GTK_VALUE_UCHAR(mapedarg[6]) = 2; // 2 Layers!
            PTGEditorWindowObjs[EO_EDIT]=MapEditClassNew(args, 7);
            mapwidget=PTGEditorWindowObjs[EO_EDIT];
            break;
        default:
            InitSelectGroup(mapeditmap,0);
            PTGEditorWindowObjs[EO_NUMGADS+int1]=MapEditClassNew(args, 11);
            mapwidget=PTGEditorWindowObjs[EO_NUMGADS+int1];
    }
    errormsg(MAPDEBUG3,"mapedit_create_new: mapwidget=%x",mapwidget);

    errormsg(MAPDEBUG1,"mapedit_create_new: Finished succesfully");
    return mapwidget;
}

void InitSelectGroup (struct MCMap *SelectGroup, gint start)
{
    gulong i,j;

    errormsg(MAPDEBUG1,"InitSelectGroup: Entered");

    SelectGroup->mm_MapSize.x = selxsize;
    SelectGroup->mm_MapSize.y = selysize;
    SelectGroup->mm_MapSize.l = 2;
    SelectGroup->mm_Copy = FALSE;
    SelectGroup->mm_Size = sizeof(struct MCMap);
    SelectGroup->mm_NextLayer = NULL;
    SelectGroup->mm_Columns = g_malloc0(sizeof(gulong)*selysize);
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

    for(i=0;i<selysize;i++)
    {
        SelectGroup->mm_Rows = g_malloc0(sizeof(struct MapPiece)*selxsize);
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

        for(j=0;j<selxsize;j++)
        {
#if DEBUGLEV > 5
            errormsg(MAPDEBUG6,"InitSelectGroup: j=%d",j);
#endif
            SelectGroup->mm_Rows[j].mp_Coordinates.x = j * mpxsize;
            SelectGroup->mm_Rows[j].mp_Coordinates.y = i * mpysize;
            SelectGroup->mm_Rows[j].mp_Coordinates.l = 1;
            SelectGroup->mm_Rows[j].mp_PixbufNumber = start+i*selysize+j;
            SelectGroup->mm_Rows[j].mp_Size = sizeof(struct MapPiece);
        }
    }
    errormsg(MAPDEBUG1,"InitSelectGroup: Finished succesfully");
}

/*
** Load and set up GUI with libglade
*/
GtkWidget *InitPTGEditorWindow( void )
{
    GtkWidget *win=NULL, *frame=NULL;
    GladeXML *xml=NULL;
    char *rootnode=NULL; // This is probably never needed

    errormsg(MAPDEBUG1,"InitPTGEditorWindow: Entered");

    glade_init();
    errormsg(MAPDEBUG4,"InitPTGEditorWindow: glade_init done");

    ngroup = 5; // Has to be changed, DB ?

    // Allocate number of GUI objects
    PTGEditorWindowObjs = g_malloc0( EO_NUMGADS + ngroup );
    {
      guint i;
      for (i=0; i <= EO_NUMGADS + ngroup ; i++)
	PTGEditorWindowObjs[i]=0;
    }

    xml = glade_xml_new(PTGMAPEDITORFILE, rootnode);

    errormsg(MAPDEBUG4,"InitPTGEditorWindow: glade_xml_new done");

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
            errormsg(MAPDEBUG3,"InitPTGEditorWindow: win=%x",win);
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
    errormsg(MAPDEBUG3,"InitPTGEditorWindow: win=%x",win);
    glade_xml_signal_autoconnect(xml);
    gtk_object_unref(GTK_OBJECT(xml));
    errormsg(MAPDEBUG3,"InitPTGEditorWindow: win=%x now!",win);

    errormsg(MAPDEBUG1,"InitPTGEditorWindow: Finished succesfully");
    return win; // Problem: We never get a window pointer ?
}
