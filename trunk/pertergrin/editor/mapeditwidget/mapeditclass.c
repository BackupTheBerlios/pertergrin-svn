/*
 * MapEditClass.c (C) 2000/2001 by Reinhard Katzmann. All rights reserved
 *
 * GTK (Gimp Tool Kit) Map Edit gadget class including possibility to select
 * mappieces so you can also create a "GetPiece" gadget.
 * It is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License,  or (at your option) any later
 * version. You should have received a copy of the GNU General Public License
 * along with this program. If not contact one the AUTHORS.
 */

//#define USE_DMALLOC 1

//#define MALLOC(size){ (malloc(size)) }
//#define ALLOC(type,size) (malloc(sizeof(type)*size))
//#define REALLOC(mem,type,size) (realloc(mem, sizeof(type)*size))

#include <sys/types.h>
// #include <sys/commsize.h>
#include <memory.h>
#include <stdlib.h>
//#include <dmalloc.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkviewport.h>
#include <gtk/gtksignal.h>
#include <gdk/gdktypes.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "mapeditclass.h" // Attention: Version numbering is here!

//#define CALLOC(type,size) (calloc(sizeof(type), size))
//#define FREE(mem) (free(mem))

extern char *getLogfile();
guint  MapEditClassGetType(void);
static void MapEditClassSet( GtkObject *obj, GtkArg *arg, guint arg_id );
static void MapEditClassGet( GtkObject *obj, GtkArg *arg, guint arg_id );
static void MapEditClassRealize (GtkWidget *widget);
static gint MapEditClassRender( GtkWidget *widget, GdkEventExpose *event );
static void MapEditClassDraw (GtkWidget *widget, GdkRectangle *area);
static void MapEditClassSizeReq (GtkWidget *widget,
				 GtkRequisition *requisition);
static void MapEditClassNotifyDimensions( GtkWidget *widget, 
				    GtkAllocation *allocation );
static gint MapEditClassNotifyMove(GtkWidget *widget, GdkEventMotion *event);
static void MapEditAdjustmentChanged( GtkAdjustment *adjustment, 
				      gpointer data);
static void MapEditAdjustmentValueChanged( GtkAdjustment *adjustment, 
					   gpointer data);
static void InitMapEditClass( GtkMapEditClass *class );
static void MapEditInit( MD *md );

GtkWidgetClass *mapedit_parent_class = NULL;

/* Debug options */
#define NOERROR 0
#define NOWARNING 0
#define NOINFO 0
#define DEBUG 1     // Set to 1 to enable debugging
#define DEBUGLEV 5       /* I: Informations only,
			  * E: Add Error messages
			  * W: Add Warning messages
			  * 1: Add entry and exit messages
			  * 2: Add important variables output
			  * 3: Add most variables output
			  * 4: Add several function calls outside
			  * 5: Add all variables output
			  * 6: Add all function calls outside
			  * 7: Add all kind of silly output
			  */

/*
**  Simple type-cast.
**/
#define GAD(x)      (( GtkWidget * )x)

#define MINROW 4
#define MINCOL 4
#define COPYMAP   1
#define COPYLAYER 2

enum {
  MAPEDIT_CURRPIECE_SIGNAL,
  MAPEDIT_GRID_SIGNAL,
  MAPEDIT_GRIDPEN_SIGNAL,
  MAPEDIT_MAP_SIGNAL,
  MAPEDIT_SELECTX_SIGNAL,
  MAPEDIT_SELECTY_SIGNAL,
  MAPEDIT_SELECTL_SIGNAL,
  LAST_SIGNAL
};

static gint mapedit_signals[LAST_SIGNAL] = { 0 };


/* errormsg - Output a message of different types like error, warning, info
 * 
 * Parameters:
 * short type:   Type of Error Message (see batbaseinterface.h)
 * char *errmsg: Message to be logged.
 * ...       : variable number of arguments as printf-type format string
 */
void errormsg(short type, char *errmsg, ...)
{

#ifdef DEBUG // Empty function if debugging disabled (bad idea ;-)
  char prmsg[1024]="",head[12]="";
  FILE *fh;

  if (!errmsg) return;

  if (errmsg[0])
  {
      va_list argp;
      va_start(argp,errmsg);
      vsnprintf(prmsg, 1011, errmsg, argp); // Max. length 1011
      va_end(argp);
  }

  switch(type)
  {
    case MAPERROR:
      if (!NOERROR) sprintf(head,"Error: ");
      else return;
      break;
    case MAPWARNING:
      if (!NOWARNING) sprintf(head,"Warning: ");
      else return;
      break;
    case MAPINFO:
      if (!NOINFO) sprintf(head,"Info: ");
      else return;
      break;
    case MAPDEBUG1:
      if (DEBUGLEV>0) sprintf(head,"Debug1: ");
      else return;
      break;
    case MAPDEBUG2:
      if (DEBUGLEV>1) sprintf(head,"Debug2: ");
      else return;
      break;
    case MAPDEBUG3:
      if (DEBUGLEV>2) sprintf(head,"Debug3: ");
      else return;
      break;
    case MAPDEBUG4:
      if (DEBUGLEV>3) sprintf(head,"Debug4: ");
      else return;
      break;
    case MAPDEBUG5:
      if (DEBUGLEV>4) sprintf(head,"Debug5: ");
      else return;
      break;
    case MAPMSG:
      if (DEBUGLEV<=4) return;
      break;
    case MAPDEBUG6:
      if (DEBUGLEV>5) sprintf(head,"Debug6: ");
      else return;
      break;
    case MAPPARANOIA:
      if (DEBUGLEV>6) sprintf(head,"Paranoia: ");
      else return;
      break;
  }

  
  if (type!=MAPMSG) 
      strcat(prmsg,"\n");

  fh=fopen(getLogfile(),"a+");
  fprintf(fh,"%s%s",head,prmsg);
  fclose(fh);
  //printf(prmsg);

#endif
}

/*
** MakeMap  - Allocates a new map and optionally makes a copy of another map
**
** guchar mkcopy        - If set, makes a copy of smap (and optionally its layers)
**                       when creating a new dmap.
** struct MCMap *dmap  - the map to be created.
** MD           *md    - pointer to object instance data.
** struct MCMap *smap  - the (optional) map to be copied.
**
** Returns: TRUE on success, FALSE otherwise
*/
static BOOL MakeMap(guchar mkcopy, struct MCMap *dmap, MD *md, 
		    struct MCMap *smap)
{
    guchar lnum = 1; // 1 Layer
    gulong i,j;
    errormsg(MAPDEBUG1,"MakeMap start");

#if DEBUGLEV > 1
    errormsg(MAPDEBUG2,"MakeMap: mkcopy=%u dmap=%x, md=%x, smap=%x",mkcopy,
	      dmap,md,smap);
#endif    
    do // Iterate through all layers
    {
        if (smap) // Default (i.e. smap!=NULL): Use default values
            dmap->mm_MapSize = smap->mm_MapSize;
        else
            dmap->mm_MapSize = md->md_Map->mm_MapSize; // Set default value
#if DEBUGLEV > 4
	errormsg(MAPDEBUG5,"MakeMap: dmap: mm_MapSize.x=%d, mm_MapSize.y=%d",
		 dmap->mm_MapSize.x, dmap->mm_MapSize.y);
#endif

#if DEBUGLEV > 5
	errormsg(MAPDEBUG6,"MakeMap: Allocating mm_Columns");
#endif
        dmap->mm_Columns = g_new0(gulong,md->md_Map->mm_MapSize.y);
#if DEBUGLEV > 2
	errormsg(MAPDEBUG3,"MakeMap: dmap=%x, md=%x, smap=%x",dmap,md,smap);
#endif
#if DEBUGLEV > 1
	errormsg(MAPDEBUG2,"MakeMap: dmap->mm_Columns=%x",dmap->mm_Columns);
#endif
        if (dmap->mm_Columns)
        {
            for (i=0; i< md->md_Map->mm_MapSize.y;i++) // All columns have to be filled
            {
#if DEBUGLEV > 5
	        errormsg(MAPDEBUG6,"MakeMap: Allocating mm_Rows");
#endif
                dmap->mm_Rows = g_malloc0(md->md_Default->mp_Size * 
					  md->md_Map->mm_MapSize.x );
#if DEBUGLEV > 5
		errormsg(MAPDEBUG6,"MakeMap: dmap=%x, md=%x, smap=%x", dmap,
			 md, smap);
#endif
#if DEBUGLEV > 1
		errormsg(MAPDEBUG2,"MakeMap: i=%u, dmap->mm_Rows=%x",i,
			 dmap->mm_Rows);
#endif
                if (dmap->mm_Rows)
                {
                    // Store allocated row in current column
                    dmap->mm_Columns[i] = (gulong)dmap->mm_Rows;
                    for (j=0; j< md->md_Map->mm_MapSize.x;j++) 
		    // Finally put MapPieces in rows
                    {
                        dmap->mm_Rows[j].mp_Coordinates.x = j * md->md_PWidth;
                        dmap->mm_Rows[j].mp_Coordinates.y = i * md->md_PLength;
                        dmap->mm_Rows[j].mp_Coordinates.l = lnum;

                        if (!smap) // Create our own map
                        {
                            dmap->mm_Rows[j].mp_Number = md->md_Default->mp_Number;
                            dmap->mm_Rows[j].mp_PixbufNumber = md->md_Default->mp_PixbufNumber;
                            dmap->mm_Rows[j].mp_Size = md->md_Default->mp_Size;
                            dmap->mm_Rows[j].mp_User = md->md_Default->mp_User;
                        }
                        else // Copy map pieces from user (he wants us to!)
                        {
                            dmap->mm_Rows[j].mp_Number = smap->mm_Rows->mp_Number;
                            dmap->mm_Rows[j].mp_PixbufNumber = smap->mm_Rows->mp_PixbufNumber;
                            dmap->mm_Rows[j].mp_Size = smap->mm_Rows->mp_Size;
                            dmap->mm_Rows[j].mp_User = smap->mm_Rows->mp_User;
                        }
                        dmap->mm_Rows[j].mp_PBCoord.x = md->md_AllPieces[dmap->mm_Rows[j].mp_PixbufNumber].mp_PBCoord.x;
                        dmap->mm_Rows[j].mp_PBCoord.y = md->md_AllPieces[dmap->mm_Rows[j].mp_PixbufNumber].mp_PBCoord.y;
                    }
                }
                else
                {
                    errormsg(MAPERROR,"MakeMap: Rows allocation failed!");
                    return FALSE; // Not enough memory
                }
            }
        }
        else
        {
	    errormsg(MAPERROR,"Makemap: Columns allocation failed!");
            return FALSE; // Not enough memory
        }
#if DEBUGLEV > 4
	errormsg(MAPDEBUG5,"MakeMap: mkcopy=%u, COPYLAYER=%u", mkcopy, 
		 COPYLAYER);
#endif
        if ( (mkcopy & COPYLAYER) && lnum < md->md_Map->mm_MapSize.l )
        {
#if DEBUGLEV > 4
	    errormsg(MAPDEBUG5,"MakeMap: mkcopy=%u, COPYMAP=%u, lnum=%u", 
		     mkcopy, COPYMAP, lnum);
#endif
            if ( (mkcopy & COPYMAP))
            /* Allocate own layer */
            {
#if DEBUGLEV > 5
	        errormsg(MAPDEBUG6,"MakeMap: Allocating mm_NextLayer");
#endif
                dmap->mm_NextLayer = g_new0(struct MCMap, 1);
#if DEBUGLEV > 4
		errormsg(MAPDEBUG5,"MakeMap: smap=%x", smap);
#endif
                if (smap) smap = smap->mm_NextLayer;
#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"MakeMap: dmap=%x, md=%x, smap=%x", dmap,
			 md, smap );
#endif
#if DEBUGLEV > 1
		errormsg(MAPDEBUG2,"MakeMap: dmap->NextLayer=%x",
			 dmap->mm_NextLayer);
#endif
                if (dmap->mm_NextLayer) dmap = dmap->mm_NextLayer;
                else
                {
		    errormsg(MAPERROR,"MakeMap: NextLayer allocation failed!");
                    return FALSE; // Not enough memory
                }
            }
#if DEBUGLEV > 4
	    errormsg(MAPDEBUG5,"MakeMap: mkcopy=%u, COPYMAP=%u", mkcopy, 
		     COPYMAP);
#endif
            if (! (mkcopy & COPYMAP))
            /* Allocate own layer only if smap has no next layer or is not valid */
            {
#if DEBUGLEV > 4
	        errormsg(MAPDEBUG5,"MakeMap: smap=%x, smap->NextLayer=%x",
			 smap,(smap ? smap->mm_NextLayer : 0));
#endif
                if (!smap || !smap->mm_NextLayer)
                {
#if DEBUGLEV > 5
		    errormsg(MAPDEBUG6,"MakeMap: Allocating mm_NextLayer");
#endif
                    dmap->mm_NextLayer = g_new0(struct MCMap, 1);
                    if (!dmap->mm_NextLayer)
                    {
		        errormsg(MAPERROR,
				 "MakeMap: NextLayer allocation failed!");
                        return FALSE; // Not enough memory
                    }
                }
                if (smap) smap = smap->mm_NextLayer;
#if DEBUGLEV > 2
                errormsg(MAPDEBUG3,"MakeMap start: dmap=%x, md=%x, smap=%x",
			 dmap, md, smap);
#endif
#if DEBUGLEV > 1
		errormsg(MAPDEBUG2," dmap->NextLayer=%x", dmap->mm_NextLayer);
#endif
                if (smap) dmap = smap->mm_NextLayer;
                else dmap = dmap->mm_NextLayer;
            }
        }
        else
        {
            dmap->mm_NextLayer = NULL;
            dmap = NULL;
        }
        if (dmap) lnum++; // See check below, lnum may not be > MapSize.l
    } while (dmap);
#if DEBUGLEV > 1
    errormsg(MAPDEBUG2,"lnum=%u, md->md_Map->mm_MapSize.l=%u", lnum, 
	     md->md_Map->mm_MapSize.l);
#endif
    if (md->md_Map->mm_MapSize.l && (lnum > md->md_Map->mm_MapSize.l)) 
    // This should not happen
    {
        errormsg(MAPERROR,"Fatal Error: Layer Size exceeded!");
        return FALSE; // Layer size exceeded, but why ??
    }
    errormsg(MAPDEBUG1,"MakeMap succesfully finished");
    return TRUE;
}

/*
** RemoveMap  - Removes a previously allocated map
**
** struct MCMap *amap  - the map to be removed
** MD           *md    - pointer to object instance data
**
** Returns: TRUE on success, FALSE otherwise.
*/
static BOOL RemoveMap( struct MCMap *amap, MD *md)
{
    guchar lnum = 1;
    errormsg(MAPDEBUG1,"RemoveMap start");
#if DEBUGLEV > 1
    errormsg(MAPDEBUG2,"RemoveMap: amap=%x, md=%x", amap, md);
#endif
    do
    {
        struct MCMap *tmap=NULL;
#if DEBUGLEV > 2
	errormsg(MAPDEBUG3,"RemoveMap: amap=%x, md=%x", amap, md);
#endif
#if DEBUGLEV > 1
	errormsg(MAPDEBUG2,"RemoveMap: amap->mm_Columns=%x", amap->mm_Columns);
#endif
#if DEBUGLEV > 4
	errormsg(MAPDEBUG5,"RemoveMap: x-Size: %u, y-Size: %u, Layer-Size: %u", 
		 amap->mm_MapSize.x, amap->mm_MapSize.y, amap->mm_MapSize.l);
#endif
        if (amap && amap->mm_Columns)
        {
            int i;
            for (i=0; i< md->md_Map->mm_MapSize.y;i++) // All columns have to be freed
            {
#if DEBUGLEV > 4
	        errormsg(MAPDEBUG5,"RemoveMap: amap=%x, md=%x, "
			 "amap->mm_Columns[%u]=%x", amap, md, i, 
			 amap->mm_Columns[i]);
#endif
                if (amap->mm_Columns[i])
                    g_free((struct MapPiece  *)amap->mm_Columns[i]);
                else break;
            }
            g_free(amap->mm_Columns);
        }
        if (lnum<md->md_Map->mm_MapSize.l) tmap = amap->mm_NextLayer; // Get pointer to next layer
#if DEBUGLEV > 4
	errormsg(MAPDEBUG5,"RemoveMap: amap=%x, tmap=%x, lnum=%u", amap, md, 
		 tmap, lnum);
#endif
        g_free(amap);        // Free this layer
        amap=tmap;            // Set new current layer
        lnum++;
    } while (amap);
    amap=NULL;
    errormsg(MAPDEBUG1,"RemoveMap succesfully finished");
    return TRUE;
}

/*
**  Initialize a new mapedit object.
**/
static void MapEditInit(MD *md)
{
  errormsg(MAPDEBUG1,"MapEditInit: Setting up md structure");

  //bzero(( char * )md, sizeof( MD ));

  /*
  **  Setup the default settings.
  **/
  md->md_Frame                      = FALSE; /* No Frame */
  md->md_FrameSpace                 = 0;
  md->md_Copy                       = TRUE;
  md->md_GetPieces                  = FALSE;
  md->md_Grid                       = FALSE; /* No Grid */
  md->md_GridPen                    = 0; /* Grid Pen Color "invisible" */
  md->md_MapPieces                  = NULL;
  md->md_Map                        = NULL;
  md->md_Default                    = NULL;
  md->md_AllPieces                  = NULL;
  md->md_Select.x = md->md_Select.y = 0;
  md->md_Select.l                   = 1;
  md->md_FrameToggle                = md->md_Select;
  md->md_ScaleWidth = md->md_ScaleHeight = 100; // 100% scaled
  md->md_UndoBuffer                 = NULL;
  md->md_InitialBuffer              = NULL;

  errormsg(MAPDEBUG1,"MapEditInit: Finished setting up md structure");
}

/*
**  Create a new mapedit object.
**/
GtkWidget *MapEditClassNew(GtkArg *args, guint num_args)
{
    MD              *md =  gtk_type_new GTK_TYPE_MAPEDIT;
    GdkPixbuf       *bm=NULL;
    struct MCMap    *amap=NULL;
    gulong          arg, i, mapx=0, mapy=0, mapl = 1;
    BOOL            mkcopy = FALSE;

    errormsg(MAPDEBUG1,"MapEditClassNew: Creating New Widget");

    /*
    **  Setup the instance data.
    **/
    errormsg(MAPDEBUG3,"MapEditClassNew: args=%x, num_args=%u",args, num_args);

    for (arg = 0 ; arg < num_args ; arg++)
    {
#if DEBUGLEV > 2
	errormsg(MAPDEBUG3,"args[%u]=%x, args[%u]->type=%u",arg,args[arg],
		 arg,args[arg].type);
#endif
        // Check all the arguments
	switch ( args[arg].type )
	{
	    case MAPEDIT_NoCopy:
	        errormsg(MAPINFO,"MapEditClassNew: Nocopy is set");
#if DEBUGLEV > 1
	        errormsg(MAPDEBUG2,"MapEditClassNew: md->md_Copy=%u",
			 md->md_Copy);
#endif
		if (GTK_VALUE_BOOL(args[arg]) && md->md_Copy)
		{
#if DEBUGLEV > 2
		    errormsg(MAPDEBUG3,"MapEditClassNew: md->md_MapPieces=%x",
			     md->md_MapPieces);
#endif
		    if (md->md_MapPieces) 
		    {
#if DEBUGLEV > 3
		        errormsg(MAPDEBUG4,"MapEditClassNew: gdk_pixbuf_unref",
				 md->md_Copy);
#endif
		        gdk_pixbuf_unref(md->md_MapPieces);
		    }
		    md->md_MapPieces = NULL;
		}
		md->md_Copy=!GTK_VALUE_BOOL(args[arg]);
#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"md->md_Copy=%u", md->md_Copy);
#endif
		break;

	    case MAPEDIT_MapPieces:
		errormsg(MAPINFO,"MapEditClassNew: MapPieces is set");
		bm = (GdkPixbuf *) GTK_VALUE_POINTER(args[arg]);
		if (md->md_Copy) {
		    if (md->md_MapPieces) gdk_pixbuf_unref(md->md_MapPieces);
		    md->md_MapPieces = NULL;
		    md->md_MapPieces=gdk_pixbuf_copy(bm);
#if DEBUGLEV > 2
		    errormsg(MAPDEBUG3,"MapPieces=%x,bm=%x",
			     md->md_MapPieces,bm);
#endif
		}
		else 
		{
		    md->md_MapPieces=gdk_pixbuf_ref(bm);
#if DEBUGLEV > 2
		    errormsg(MAPDEBUG3,"MapPieces=%x", md->md_MapPieces);
#endif
		}
#if DEBUGLEV > 3
		    errormsg(MAPDEBUG4,"Width=%u, Height=%u, Rowstride=%u", 
			     gdk_pixbuf_get_width(md->md_MapPieces), 
			     gdk_pixbuf_get_height(md->md_MapPieces),
			     gdk_pixbuf_get_rowstride(md->md_MapPieces));
#endif
		break;

	    case MAPEDIT_PWidth:
                errormsg(MAPINFO,"MapEditClassNew: PWidth is set");
		md->md_PWidth = GTK_VALUE_UINT(args[arg]);
#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"PWidth=%u", md->md_PWidth);
#endif
		break;

            case MAPEDIT_PLength:
                errormsg(MAPINFO,"MapEditClassNew: PLength is set");
		md->md_PLength = GTK_VALUE_UINT(args[arg]);
#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"PLength=%u", md->md_PLength);
#endif
		break;

	    case MAPEDIT_Map:
                errormsg(MAPINFO,"MapEditClassNew: Map is set");
		amap = (struct MCMap *) GTK_VALUE_POINTER(args[arg]);
		if (amap->mm_Copy) mkcopy = TRUE;
		break;

	    case MAPEDIT_MapWidth:
                errormsg(MAPINFO,"MapEditClassNew: MapWidth is set");
		mapx = GTK_VALUE_UINT(args[arg]);
#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"MapWidth=%u", mapx);
#endif
		break;

	    case MAPEDIT_MapLength:
                errormsg(MAPINFO,"MapEditClassNew: MapLength is set");
		mapy = GTK_VALUE_UINT(args[arg]);
#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"MapLength=%u", mapy);
#endif
		break;

	    case MAPEDIT_MapLayer:
                errormsg(MAPINFO,"MapEditClassNew: MapLayer is set");
		mapl = GTK_VALUE_UCHAR(args[arg]);
#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"MapLayer=%u", mapl);
#endif
		break;

	    case MAPEDIT_GetPieces:
                errormsg(MAPINFO,"MapEditClassNew: GetPieces is set");
		md->md_GetPieces = GTK_VALUE_BOOL(args[arg]);
#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"GetPieces=%u", md->md_GetPieces);
#endif
		break;

	    case MAPEDIT_Frame:
                errormsg(MAPINFO,"MapEditClassNew: Frame is set");
		md->md_Frame = GTK_VALUE_BOOL(args[arg]);
#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"Frame=%u", md->md_Frame);
#endif
		break;

	    case MAPEDIT_FrameSpace:
                errormsg(MAPINFO,"MapEditClassNew: FrameSpace is set");
		md->md_FrameSpace = GTK_VALUE_UCHAR(args[arg]);
#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"FrameSpace=%u", md->md_FrameSpace);
#endif
		break;

	    case MAPEDIT_CurrPiece:
                errormsg(MAPINFO,"MapEditClassNew: CurrPiece is set");
		md->md_CurrPiece.mp_Number = ((struct MapPiece *)GTK_VALUE_POINTER(args[arg]))->mp_Number;
		md->md_CurrPiece.mp_PixbufNumber = ((struct MapPiece *)GTK_VALUE_POINTER(args[arg]))->mp_PixbufNumber;
		md->md_CurrPiece.mp_PBCoord = ((struct MapPiece *)GTK_VALUE_POINTER(args[arg]))->mp_PBCoord;
		md->md_CurrPiece.mp_Size = ((struct MapPiece *)GTK_VALUE_POINTER(args[arg]))->mp_Size;
		md->md_CurrPiece.mp_User = ((struct MapPiece *)GTK_VALUE_POINTER(args[arg]))->mp_User;
                break;

	    case MAPEDIT_Default:
                errormsg(MAPINFO,"MapEditClassNew: Default is set");
		md->md_Default = (struct MapPiece *) GTK_VALUE_POINTER(args[arg]);
		break;

	    case MAPEDIT_Grid:
                errormsg(MAPINFO,"MapEditClassNew: Grid is set");
		md->md_Grid = GTK_VALUE_BOOL(args[arg]);
		break;

	    case MAPEDIT_GridPen:
                errormsg(MAPINFO,"MapEditClassNew: GridPen is set");
		md->md_GridPen = GTK_VALUE_UCHAR(args[arg]);
		break;

	    case MAPEDIT_SelectX:
	        errormsg(MAPINFO,"MapEditClassNew: SelectX is set");
		md->md_Select.x = GTK_VALUE_UINT(args[arg]);
		break;

	    case MAPEDIT_SelectY:
	        errormsg(MAPINFO,"MapEditClassNew: SelectY is set");
		md->md_Select.y = GTK_VALUE_UINT(args[arg]);
		break;

	    case MAPEDIT_SelectL:
	        errormsg(MAPINFO,"MapEditClassNew: SelectL is set");
		md->md_Select.l = GTK_VALUE_UCHAR(args[arg]);
		break;

	    case MAPEDIT_ScaleWidth:
	        errormsg(MAPINFO,"MapEditClassNew: ScaleWidth is set");
		md->md_ScaleWidth = GTK_VALUE_UCHAR(args[arg]);
		break;

	    case MAPEDIT_ScaleHeight:
	        errormsg(MAPINFO,"MapEditClassNew: ScaleHeight is set");
		md->md_ScaleHeight = GTK_VALUE_UCHAR(args[arg]);
		break;
	}
    }

    /*
    **  Test if we have all attributes
    **  and allocate the map
    */
#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"MapPieces=%x,md->md_PWidth=%u, md->md_PLength=%u,"
	     "md->md_Default=%x",md->md_MapPieces,md->md_PWidth,
	     md->md_PLength, md->md_Default);
#endif
    if (md->md_MapPieces && md->md_PWidth && md->md_PLength && md->md_Default)
    {
#if DEBUGLEV > 2
        errormsg(MAPDEBUG3,"mapx=%u,mapy=%u,MINROW=%u,MINCOL=%u",
		 mapx, mapy, MINROW, MINCOL);
#endif
        if (mapx >= MINROW && mapy >= MINCOL)
	{
#if DEBUGLEV > 2
	    errormsg(MAPDEBUG3,"amap=%x, mkcopy=%u",amap,mkcopy);
#endif
	    if (!amap || mkcopy)
	    {
	        md->md_Map = g_malloc0(sizeof(struct MCMap));
	    }
	    if (!mkcopy && amap)
            {
	        md->md_Map = amap;
	    }
#if DEBUGLEV > 2
	    errormsg(MAPDEBUG3,"md->md_Map=%x",md->md_Map);
#endif
	    if (md->md_Map)
	    {
	        guint wtn, htn;
		BOOL ret=FALSE;

		// Calculate pixbuf coordinates
                wtn = gdk_pixbuf_get_width(md->md_MapPieces) / md->md_PWidth;
	        htn = gdk_pixbuf_get_height(md->md_MapPieces) / md->md_PLength;
#if DEBUGLEV > 1
		errormsg(MAPDEBUG2,"wtn=%u,htn=%u",wtn, htn);
#endif

		if (md->md_Default->mp_Number==0)
		{ // Calculate number of map pieces
		    md->md_Default->mp_Number = wtn *  htn;
		}

		if(md->md_Default->mp_Size==0)
		    md->md_Default->mp_Size = sizeof(struct MapPiece);

#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"md->md_Default->mp_Number=%u,"
			 "md->md_Default->mp_PixbufNumber=%u",
			 md->md_Default->mp_Number,
			 md->md_Default->mp_PixbufNumber);
#endif
		if(md->md_Default->mp_Number > md->md_Default->mp_PixbufNumber)
		{
		    md->md_AllPieces = g_malloc0(sizeof(struct MapPiece)*md->md_Default->mp_Number);
#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"md->md_AllPieces=%x",md->md_AllPieces);
#endif

		    for (i=0;i<md->md_Default->mp_Number;i++)
		    {
		      md->md_AllPieces[i].mp_PBCoord.x = (i%wtn)*md->md_PWidth;
		      md->md_AllPieces[i].mp_PBCoord.y = (i/wtn)*md->md_PLength;
		    }

		    if (!amap || mkcopy)
		    {
		        if (md->md_AllPieces)
			{
			    struct MCMap *tmap = md->md_Map;
			    md->md_Map->mm_MapSize.x = mapx;
			    md->md_Map->mm_MapSize.y = mapy;
			    md->md_Map->mm_MapSize.l = mapl; /* No additional Layers by default */
			    md->md_Map->mm_Copy = COPYMAP; /* Important! We need this flag to free memory again! */
			    mkcopy |= COPYLAYER;
#if DEBUGLEV > 2
			  errormsg(MAPDEBUG3,"mkcopy=%u,tmap=%x,md=%x,amap=%x",
				   mkcopy, tmap, md, amap);
#endif
			    ret=MakeMap(mkcopy,tmap,md,amap);
			}
		    }
		    else
		    {
		        do // Iterate through all layers
			{
			    gulong i,j;
			    for (i=0; i< md->md_Map->mm_MapSize.y;i++) // All columns have to be filled
			    {
			        amap->mm_Rows = (void *) amap->mm_Columns[i];
#if DEBUGLEV > 2
				errormsg(MAPDEBUG3,"amap->mm_Rows=%x,"
					 "amap->mm_Columns[%u]=%x",
					 amap->mm_Rows,i,amap->mm_Columns[i]);
#endif
				for (j=0; j< md->md_Map->mm_MapSize.x;j++) // Calculate MapPieces coordinates
				{
				    amap->mm_Rows[j].mp_PBCoord.x = md->md_AllPieces[amap->mm_Rows[j].mp_PixbufNumber].mp_PBCoord.x;
				    amap->mm_Rows[j].mp_PBCoord.y = md->md_AllPieces[amap->mm_Rows[j].mp_PixbufNumber].mp_PBCoord.y;
#if DEBUGLEV > 3
				    errormsg(MAPDEBUG4,"[%u].mp_PBCoord.x=%u, "
					     "[%u].mp_PBCoord.y=%u,",
					     j,amap->mm_Rows[j].mp_PBCoord.x,
					     j,amap->mm_Rows[j].mp_PBCoord.y);
#endif
				}
			    }
			    amap = amap->mm_NextLayer;
			}
			while(amap);
			ret = TRUE;
		    }

		    if (ret)
		    {
		        md->md_FrameToggle.x = ((struct MapPiece *)md->md_Map->mm_Columns[0])->mp_Coordinates.x;
		        md->md_FrameToggle.y = ((struct MapPiece *)md->md_Map->mm_Columns[0])->mp_Coordinates.y;
			md->md_FrameToggle.l = 1;
			errormsg(MAPDEBUG1,"MapEditClassNew: Finished!");
			return GTK_WIDGET ( md );
    
		    }
		}
	    }
	}
    }
    errormsg(MAPDEBUG1,"MapEditClassNew: Failed!");
    return NULL;
}

/*
**  Dispose of the object.
**/
static void MapEditClassDispose( GtkObject *obj )
{
    MD          *md = ( MD * ) GTK_MAPEDIT_SELECT( obj );

    /*
    **  Free Pic Area & Undo Buffer
    **/
    errormsg(MAPDEBUG1,"MapEditClassDipose started");
    
    if (md->md_Copy && md->md_MapPieces) {
        gdk_pixbuf_unref(md->md_MapPieces);
        md->md_MapPieces=NULL;
    }
#if DEBUGLEV > 1
    errormsg(MAPDEBUG2,"Map=%x, mm_Copy=%x, md=%x",
	     md->md_Map, md->md_Map->mm_Copy, md);
#endif
    if (md->md_Map && (md->md_Map->mm_Copy & COPYMAP))
        RemoveMap(md->md_Map,md);
    if (md->md_UndoBuffer)
        RemoveMap(md->md_UndoBuffer,md);
    if (md->md_InitialBuffer)
        RemoveMap(md->md_InitialBuffer,md);
    if (md->md_AllPieces) {
        g_free(md->md_AllPieces);
        md->md_AllPieces=NULL;
    }

    /*
    **  The superclass handles
    **  the rest.
    **/
    if (GTK_OBJECT_CLASS(mapedit_parent_class)->destroy)
      (* (GTK_OBJECT_CLASS(mapedit_parent_class)->destroy))(obj);

    errormsg(MAPDEBUG1,"MapEditClassDipose finished");
}

/*
**  Get an attribute.
*/
static void MapEditClassGet( GtkObject *obj, GtkArg *arg, guint arg_id )
{
    MD          *md = ( MD * ) GTK_MAPEDIT_SELECT( obj );
    gulong       i;
    struct MCMap *amap=NULL;

    /*
    **  Handle our attribute.
    **/
    switch ( arg->type )
    {
        case MAPEDIT_Map:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: Map=%x, mm_Copy=%x, md=%x",
	     md->md_Map, md->md_Map->mm_Copy, md);
#endif
            if (md->md_Map->mm_Copy) // Always get a copy of our map!
            {
                amap = (struct MCMap *) GTK_VALUE_POINTER(*arg);
                for (i=0; i< (md->md_Map->mm_MapSize.x * md->md_Map->mm_MapSize.y * (md->md_Map->mm_MapSize.l ? md->md_Map->mm_MapSize.l : 1)); i++)
                amap[i]=md->md_Map[i];
            }
            else  // Special case: We are not the owner of the map!
            {
                GTK_VALUE_POINTER(*arg) = md->md_Map;
            }
            break;

        case MAPEDIT_MapWidth:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: Map=%x, mm_MapSize.x=%u, md=%x",
		   md->md_Map, md->md_Map->mm_MapSize.x, md);
#endif
            GTK_VALUE_UINT(*arg) = md->md_Map->mm_MapSize.x;
            break;

        case MAPEDIT_MapLength:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: Map=%x, mm_MapSize.y=%u, md=%x",
		   md->md_Map, md->md_Map->mm_MapSize.y, md);
#endif
            GTK_VALUE_UINT(*arg) = md->md_Map->mm_MapSize.y;
            break;

        case MAPEDIT_MapLayer:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: Map=%x, mm_MapSize.y=%u, md=%x",
		   md->md_Map, md->md_Map->mm_MapSize.l, md);
#endif
            GTK_VALUE_UCHAR(*arg) = md->md_Map->mm_MapSize.l;
            break;

        case MAPEDIT_CurrPiece:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: CurrPiece=%x, md=%x",
		   &md->md_CurrPiece, md);
#endif
            GTK_VALUE_POINTER(*arg) = &md->md_CurrPiece;
            break;

        case MAPEDIT_Grid:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: md->md_Grid=%u, md=%x",
		   md->md_Grid, md);
#endif
            GTK_VALUE_BOOL(*arg) = md->md_Grid;
            break;

        case MAPEDIT_GridPen:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: md->md_GridPen=%u, md=%x",
		   md->md_GridPen, md);
#endif
            GTK_VALUE_UCHAR(*arg) = md->md_GridPen;
            break;

        case MAPEDIT_SelectX:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: md->md_Select.x=%u, md=%x",
		   md->md_Select.x, md);
#endif
            GTK_VALUE_UINT(*arg) = md->md_Select.x;
            break;

        case MAPEDIT_SelectY:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: md->md_Select.y=%u, md=%x",
		   md->md_Select.y, md);
#endif
            GTK_VALUE_UINT(*arg) = md->md_Select.y;
            break;

        case MAPEDIT_SelectL:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: md->md_Select.l=%u, md=%x",
		   md->md_Select.l, md);
#endif
            GTK_VALUE_UCHAR(*arg) = md->md_Select.l;
            break;

        default:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: Default, md=%x",md);
#endif
        break;
    }
}

/*
** Create X Window on screen
**/
static void MapEditClassRealize (GtkWidget *widget)
{
    MD *md;
    errormsg(MAPDEBUG1,"MapEditClassRealize entered");

#if DEBUGLEV > 2
	        errormsg(MAPDEBUG3,"widget=%x",widget);
#endif
    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_MAPEDIT (widget));

    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
    md = GTK_MAPEDIT_SELECT(widget);
#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"md=%x",md);
#endif
    if (widget->parent)
    {
        GdkEventMask emask;
	GtkViewport *vp;
        errormsg(MAPDEBUG1,"MapEditClassRealize parent");

	// We require a viewport for proper operation
	g_return_if_fail ( GTK_IS_VIEWPORT ( widget->parent ));

        widget->window = gtk_widget_get_parent_window (widget);
	emask = gdk_window_get_events( widget->window );
	gdk_window_set_events( widget->window, emask | GDK_BUTTON_PRESS_MASK | 
	  GDK_BUTTON_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK | GDK_EXPOSURE_MASK 
	  | GDK_LEAVE_NOTIFY_MASK | GDK_PROPERTY_CHANGE_MASK );
        gdk_window_ref (widget->window);

	vp = GTK_VIEWPORT ( widget->parent );
	md->hadjustment = vp->hadjustment;
	md->vadjustment = vp->vadjustment;

	gtk_signal_connect (GTK_OBJECT (md->hadjustment), "changed",
			    (GtkSignalFunc) MapEditAdjustmentChanged,
			    (gpointer) md);
	gtk_signal_connect (GTK_OBJECT (md->hadjustment), "value_changed",
			    (GtkSignalFunc) MapEditAdjustmentValueChanged,
			    (gpointer) md);

	gtk_signal_connect (GTK_OBJECT (md->vadjustment), "changed",
			    (GtkSignalFunc) MapEditAdjustmentChanged,
			    (gpointer) md);
	gtk_signal_connect (GTK_OBJECT (md->vadjustment), "value_changed",
			    (GtkSignalFunc) MapEditAdjustmentValueChanged,
			    (gpointer) md);
    }
    else // Create own window (problem: whole map always visible)
    {
        GdkWindowAttr attributes;
	gint attributes_mask;

        errormsg(MAPDEBUG1,"MapEditClassRealize new");

	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.window_type = GDK_WINDOW_CHILD;
	// Add events which interests us
	attributes.event_mask = gtk_widget_get_events (widget) | 
	  GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | 
	  GDK_BUTTON_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK | 
	  GDK_LEAVE_NOTIFY_MASK | GDK_PROPERTY_CHANGE_MASK;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);

	attributes_mask = GDK_WA_X | GDK_WA_Y| GDK_WA_VISUAL | GDK_WA_COLORMAP;
	widget->window = gdk_window_new (widget->parent->window, &attributes, 
					 attributes_mask);
    }
    gdk_window_set_user_data (widget->window, widget);
    widget->style = gtk_style_attach (widget->style, widget->window);
    gtk_style_set_background (widget->style, widget->window, 
			      GTK_STATE_NORMAL);

    errormsg(MAPDEBUG1,"MapEditClassRealize finished");
}

/* 
** Tell'm our minimum dimensions. (Compute real size of widget)
** Attention: This is virtually ever larger than the size of the screen
** which means that a viewport is required for using this widget!
**/
static void MapEditClassSizeReq (GtkWidget *widget,
				 GtkRequisition *requisition)
{
    MD *md;
    gulong width=0, height=0, w=0, h=0;

    errormsg(MAPDEBUG1,"MapEditClassSizeReq entered");

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_MAPEDIT (widget));
    g_return_if_fail (requisition != NULL);

#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"widget=%x, width=%u, height=%u",widget,width,height);
#endif
    md = GTK_MAPEDIT_SELECT(widget);

    // Basic: Width/Height + Frame
    w = md->md_Frame ? widget->style->klass->xthickness+md->md_FrameSpace : 0;
    h = md->md_Frame ? widget->style->klass->ythickness+md->md_FrameSpace : 0;
    width = md->md_Map->mm_MapSize.x * (md->md_PWidth+w);
    height = md->md_Map->mm_MapSize.y * (md->md_PLength+h);

    // Scale factor (%)
    width=(width*md->md_ScaleWidth)/100;
    height=(height*md->md_ScaleHeight)/100;
#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"Now width=%u, height=%u",width,height);
#endif
    if ((width+MINCOL) > MINCOL) width-=MINCOL;
    if ((height+MINROW) > MINROW) height-=MINROW;

    requisition->width = MINROW + width + widget->style->klass->xthickness;
    requisition->height = MINCOL + height + widget->style->klass->ythickness;

    errormsg(MAPDEBUG1,"MapEditClassSizeReq finished");
}

/*
**  Render the Pixbuf of our object
**/
static BOOL RenderPixbuf( MD *md, GtkAllocation *area)
{
    guint       colsize = 0, rowsize = 0;
    guint       left, top, xpos, ypos, selx, sely; //, color=0;
    gulong       cx, cy, i, j, fw = 0, fh = 0, ysize;
    /* static */ struct MCMap *amap = NULL; // Hmm, why did I want it static ??

    errormsg(MAPDEBUG1,"RenderPixbuf: Entered");

#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"RenderPixbuf: md=%x, area=%x",md,area);
#endif

#if DEBUGLEV > 3
    errormsg(MAPDEBUG4,"RenderPixbuf: area->x=%u, area->y=%u, area->width=%u, "
	     "area->height=%u, name=%s, parent name=%s",
	     area->x, area->y, area->width, area->height, 
	     md->child.name, md->child.parent->name);
#endif

    /*
    **  Get initial left and top offset.
    **/
    if (!amap) amap = md->md_Map; // This makes of course only sense if static
    if (!amap) return FALSE;
    left = area->x + 1;
    top  = area->y + 1;
    if (md->md_GetPieces && md->md_Frame)
    {
        left+=md->md_FrameSpace;
        top+=md->md_FrameSpace;
    }

#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"RenderPixbuf: amap=%x, left=%u, top=%u",amap,left,top);
#endif

    /*
    **  Calculate real Width and Height of the area to blit. This depends
    **  on the width and height of the map pieces and on the Grid
    **  or on the frame thickness and the frame size instead of the Grid;
    */

    if (md->md_GetPieces)
    {
        if (md->md_Frame)
        {
	    //DoMethod( md->md_Frame, OM_GET, FRM_FrameWidth,  &fw );
	    //DoMethod( md->md_Frame, OM_GET, FRM_FrameHeight, &fh );
	    fw = md->child.style->klass->xthickness;
	    fh = md->child.style->klass->ythickness;
#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"RenderPixbuf: fw=%u, fh=%u",fw,fh);
#endif
            fw+=1+md->md_FrameSpace;
            fh+=1+md->md_FrameSpace;
            cx=(area->width/(md->md_PWidth+fw))*(md->md_PWidth+fw);
            cy=(area->height/(md->md_PLength+fh))*(md->md_PLength+fh);
#if DEBUGLEV > 3
    errormsg(MAPDEBUG4,"Renderpixbuf: Now fw=%u, fh=%u, cx=%u, cy=%u",
	     fw, fh, cx, cy);
#endif
        }
        else
        {
            cx = (area->width/md->md_PWidth)*md->md_PWidth;
            cy = (area->height/md->md_PLength)*md->md_PLength;
	    fw = fh = 1;
#if DEBUGLEV > 3
    errormsg(MAPDEBUG4,"Renderpixbuf: cx=%u, cy=%u", cx, cy);
#endif
        }
    }
    else
    {
        cx = (area->width/(md->md_PWidth+(md->md_Grid ? 1 : 0)))*(md->md_PWidth+(md->md_Grid ? 1 : 0));
        cy = (area->height/(md->md_PLength+(md->md_Grid ? 1 : 0)))*(md->md_PLength+(md->md_Grid ? 1 : 0));
	if (md->md_Grid) fw = fh = 1;
#if DEBUGLEV > 3
    errormsg(MAPDEBUG4,"Renderpixbuf: cx=%u, cy=%u", cx, cy);
#endif
    }

    /*
    ** Select the correct Map layer
    */
    if ( md->md_Map->mm_MapSize.l &&
        ((struct MapPiece *)amap->mm_Columns[0])->mp_Coordinates.l != md->md_Select.l )
    {
        int lnum = md->md_Select.l;  // Currently selected layer
        if (lnum > ((struct MapPiece *)amap->mm_Columns[0])->mp_Coordinates.l)
        {
            amap = md->md_Map;
            lnum = 1;
        }
        while (lnum!=md->md_Map->mm_MapSize.l)
        {
            if (amap->mm_NextLayer)
            {
                amap = amap->mm_NextLayer;
            }
            else
            {
                return(FALSE);
            }
            lnum++;
        }
#if DEBUGLEV > 3
	errormsg(MAPDEBUG4,"Renderpixbuf: layer=%u, amap=%x", lnum, amap);
#endif
    }
    /*
    **  Calculate absolute selected map position
    */
    selx = md->md_Select.x * md->md_PLength;
    sely = md->md_Select.y * md->md_PWidth;
    ysize = 0; //md->md_Default->mp_Size * md->md_Select.y;
#if DEBUGLEV > 3
    errormsg(MAPDEBUG4,"Renderpixbuf: selx=%u, sely=%u, ysize=%u", selx, sely,
	     ysize);
#endif

    /*
    **  Calculate width and height of the section to be blitted
    **  including relative selected map position
    */
    rowsize = cx/md->md_PWidth + md->md_Select.x;
    if (rowsize > amap->mm_MapSize.x) rowsize = amap->mm_MapSize.x;
    colsize = cy/md->md_PLength + md->md_Select.y;
    if (colsize > amap->mm_MapSize.y) colsize = amap->mm_MapSize.y;
#if DEBUGLEV > 3
    errormsg(MAPDEBUG4,"Renderpixbuf: rowsize=%u, colsize=%u, mapx=%u, mapy=%u"
	     , rowsize, colsize, amap->mm_MapSize.x, amap->mm_MapSize.y);
#endif

    /*
    **  Copy map pieces into temporary Pixbuf based on the section of the Map
    **  TODO: First find the correct layer (Maybe only in MAPEDIT_SelectL !!)
    */
    for (i=md->md_Select.y;i<colsize;i++)
    {
        guint mi = i-md->md_Select.y;
        amap->mm_Rows = (void *) (amap->mm_Columns[i] + ysize);
	errormsg(MAPINFO,"coord[0].x=%u, coord[0].y=%u, Row=%x", 
		 amap->mm_Rows[0].mp_Coordinates.x,
		 amap->mm_Rows[0].mp_Coordinates.y, amap->mm_Rows-ysize);
        for(j=md->md_Select.x;j<rowsize;j++)
        {
	    guint mj = j-md->md_Select.x;

	    // Scale Map Piece before drawing
	    // Thinkable alternative:
	    // Scale md->md_MapPieces completely when Scale is changed
	    // Disadvantage: Might cost much memory.
	    if (md->md_ScaleWidth !=100 || md->md_ScaleHeight != 100)
	    {
	        guint scx = md->md_PWidth * md->md_ScaleWidth/100,
		       scy = md->md_PLength * md->md_ScaleHeight/100;
		GdkPixbuf *tmppb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE,
						  8, md->md_PWidth, 
						  md->md_PLength );
		GdkPixbuf *tpb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE,
						8, scx, scy );

		xpos = amap->mm_Rows[j].mp_Coordinates.x * 
		  md->md_ScaleWidth/100 - selx + mj*2*fw;
		ypos = amap->mm_Rows[j].mp_Coordinates.y * 
		  md->md_ScaleHeight/100 -sely + mi*2*fh;

		// Copy map piece to be scaled
		gdk_pixbuf_copy_area(md->md_MapPieces,
				     amap->mm_Rows[j].mp_PBCoord.x,
				     amap->mm_Rows[j].mp_PBCoord.y, 
				     md->md_PWidth, md->md_PLength, tmppb,
				     0, 0);

		// Scale it to the other temporary buffer
		gdk_pixbuf_scale(tmppb, tpb, 0, 0, scx, scy,
				 0, 0, md->md_ScaleWidth/100, 
				 md->md_ScaleHeight/100, GDK_INTERP_BILINEAR);

		// Render it on window
		gdk_pixbuf_render_to_drawable(tpb, md->child.window,
                                  md->child.style->fg_gc[GTK_STATE_NORMAL],
				  0, 0, xpos, ypos, scx, scy,
                                  GDK_RGB_DITHER_NORMAL, xpos, ypos);

		// Free the pixbufs
		gdk_pixbuf_unref(tpb);
		gdk_pixbuf_unref(tmppb);
	    }
	    else // No scaling
	    {
	      xpos = amap->mm_Rows[j].mp_Coordinates.x - selx + mj*2*fw;
	      ypos = amap->mm_Rows[j].mp_Coordinates.y - sely + mi*2*fh;

	      gdk_pixbuf_render_to_drawable(md->md_MapPieces, md->child.window,
                                  md->child.style->fg_gc[GTK_STATE_NORMAL],
                                  amap->mm_Rows[j].mp_PBCoord.x,
                                  amap->mm_Rows[j].mp_PBCoord.y, xpos, ypos,
                                  md->md_PWidth, md->md_PLength, 
                                  GDK_RGB_DITHER_NORMAL, xpos, ypos);
	    }

#if DEBUGLEV > 3
	    errormsg(MAPDEBUG4,"Renderpixbuf: i=%u, j=%u, xc=%u, yc=%u, xp=%u,"
		     "yp=%u", i, j, md->md_Map->mm_Rows[j].mp_PBCoord.x, 
		     amap->mm_Rows[j].mp_PBCoord.y, xpos, ypos);
#endif
            if (md->md_GetPieces)
            {
                if (md->md_Frame)
                {
		    gtk_paint_shadow (md->child.style, md->child.window,
				      GTK_STATE_NORMAL, GTK_SHADOW_OUT,
				      NULL, &md->child, "text",
				      xpos, ypos,
				      md->md_PWidth, md->md_PLength);
                }

            }
            else
            {
                /* Draw Grid if wished */
                if (md->md_Grid)
                {
		    errormsg(MAPINFO,"Grid");
                    if (!xpos%cx) 
		        gdk_draw_rectangle(md->child.window, 
					   md->child.style->black_gc,
					   TRUE, left, ypos,
					   left+area->width-1, ypos);

                    if (!ypos%cy) 
		        gdk_draw_rectangle(md->child.window, 
					   md->child.style->black_gc,
					   TRUE, xpos, top,
					   xpos, top+area->height-1);
#if DEBUGLEV > 3
		    errormsg(MAPDEBUG4,"Renderpixbuf: Grid at %u,%u-%u,%u and"
			     " %u,%u-%u,%u", left, ypos, left+area->width-1, 
			     ypos, xpos, top, xpos, top+area->height-1);
#endif
                }
            }
        }
    }

    // Draw the toggle gadget
    if ( md->md_Frame && md->md_FrameToggle.l == md->md_Select.l &&
        (md->md_FrameToggle.x-selx) < cx &&
        (md->md_FrameToggle.y-sely) < cy )
    {
        /*
        **  Render the "frame" so it looks like a toggled widget.
        **/
        guint ftx = md->md_FrameToggle.x * md->md_ScaleWidth/100 - selx,
	  fty = md->md_FrameToggle.y * md->md_ScaleHeight/100 - sely;

        gdk_draw_rectangle(md->child.window, md->child.style->black_gc,
			   FALSE, ftx, fty, md->md_PWidth, md->md_PLength);

	gtk_paint_shadow (md->child.style, md->child.window,
			  GTK_STATE_NORMAL, GTK_SHADOW_IN, NULL, &md->child,
			  "text", ftx, fty, md->md_PWidth, md->md_PLength);
    }
    /*
    **  Free temporary Pixbuf
    */
    errormsg(MAPDEBUG1,"RenderPixbuf: Finished succesfully");
    return TRUE;
}

/*
**  Notify about an attribute change.
**/
static void NotifyAttrChange( GtkObject *obj, char *value )
{
    errormsg(MAPDEBUG1,"NotifyAttrChange: Emitting \"%s\" signal",value);
    gtk_signal_emit_by_name (obj, value);
}

/*
**  Render (GTK: Expose) the mapedit object.
**/
static gint MapEditClassRender( GtkWidget *widget, GdkEventExpose *event )
{
    MD              *md = ( MD * ) GTK_MAPEDIT_SELECT( widget );
    GtkAllocation   myalloc;

    errormsg(MAPDEBUG1,"Expose: Entered");

    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_MAPEDIT (widget), FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

#if DEBUGLEV > 3
    errormsg(MAPDEBUG4,"Expose: event->area.x=%u, .y=%u, width=%u, height=%u,"
	     "widget.x=%u, .y=%u, width=%u, height=%u", event->area.x, 
	     event->area.y, event->area.width, event->area.height, 
	     md->child.allocation.x, md->child.allocation.y, 
	     md->child.allocation.width, md->child.allocation.height);
#endif
    // Do not render more than once (but pretend you just had)
    if (event->count > 0)
        return FALSE;

    /*
    **  Render the pixel rectangles.
    **  Actually only render the visible part, else everything gets to slow
    **/
    myalloc.x = event->area.x;
    myalloc.y = event->area.y;
    if (widget->parent)
    {
	myalloc.width = md->md_PWidth + widget->parent->allocation.width;
	myalloc.height = md->md_PLength + widget->parent->allocation.height;
    }
    else
    {
	myalloc.width = md->md_PWidth + widget->allocation.width;
	myalloc.height = md->md_PLength + widget->allocation.height;
    }
    if (!RenderPixbuf( md, &myalloc ))
      return FALSE; /* Don't know what to return in case this fails */

    errormsg(MAPDEBUG1,"Expose: Finished succesfully");
    return FALSE;
}

/*
 * Draw the mapedit widget (simple function)
 * Not really necessary, expose should be enough
 */
static void MapEditClassDraw (GtkWidget *widget, GdkRectangle *area)
{
    MD              *md = ( MD * ) GTK_MAPEDIT_SELECT( widget );
    GtkAllocation   myalloc;

    errormsg(MAPDEBUG1,"Draw: Entered");
#if DEBUGLEV > 3
    errormsg(MAPDEBUG4,"Draw: area->x=%u, area->y=%u, area->width=%u, "
	     "area->height=%u, name=%s, parent name=%s",
	     area->x, area->y, area->width, area->height, 
	     md->child.name, md->child.parent->name);
#endif
    myalloc.x = area->x;
    myalloc.y = area->y;
    myalloc.width = md->md_PWidth + area->width;
    myalloc.height = md->md_PLength + area->height;
    RenderPixbuf( md, &myalloc );
    errormsg(MAPDEBUG1,"Draw: Finished succesfully");
}

/*
**  Draw a piece with the currently selected map piece.
**/
static gulong DrawPiece( MD *md, GtkAllocation *area, COORD newpiece)
{
    COORD ppos, len;
    struct MCMap *amap = md->md_Map;

    errormsg(MAPDEBUG1,"DrawPiece: Entered");

    len.x = md->md_PWidth + md->md_Grid ? 1 : 0;
    len.y = md->md_PLength + md->md_Grid ? 1 : 0;
    ppos.x = (newpiece.x/len.x) * len.x;
    ppos.y = (newpiece.y/len.y) * len.y;

    if (!amap) return( MAPERR_NoMap );
    if (md->md_Map->mm_MapSize.l)
    {
        int lnum = 1;
        while (lnum!=md->md_Select.l)
        {
            if (amap->mm_NextLayer)
            {
                amap = amap->mm_NextLayer;
            }
            else
            {
                return(MAPERR_UnknownLayer);
            }
            lnum++;
        }
    }
    else if (md->md_Select.l)
    {
        return(MAPERR_NoLayer);
    }
    amap->mm_Rows = (void *) amap->mm_Columns[md->md_Select.y];
    amap->mm_Rows[md->md_Select.x].mp_PixbufNumber = md->md_CurrPiece.mp_PixbufNumber;
    amap->mm_Rows[md->md_Select.x].mp_PBCoord.x = md->md_AllPieces[amap->mm_Rows[md->md_Select.x].mp_PixbufNumber].mp_PBCoord.x;
    amap->mm_Rows[md->md_Select.x].mp_PBCoord.y = md->md_AllPieces[amap->mm_Rows[md->md_Select.x].mp_PixbufNumber].mp_PBCoord.y;


    gdk_pixbuf_render_to_drawable(md->md_MapPieces, md->child.window,
				  md->child.style->fg_gc[GTK_STATE_NORMAL],
				  amap->mm_Rows[md->md_Select.x].mp_PBCoord.x,
				  amap->mm_Rows[md->md_Select.x].mp_PBCoord.y,
				  ppos.x, ppos.y, md->md_PWidth, 
				  md->md_PLength, GDK_RGB_DITHER_NORMAL,
				  amap->mm_Rows[md->md_Select.x].mp_PBCoord.x,
				  amap->mm_Rows[md->md_Select.x].mp_PBCoord.y);

    md->md_CurrPiece.mp_Coordinates.l = md->md_Select.l;
    md->md_CurrPiece.mp_Coordinates.x = amap->mm_Rows[md->md_Select.x].mp_Coordinates.x;
    md->md_CurrPiece.mp_Coordinates.y = amap->mm_Rows[md->md_Select.x].mp_Coordinates.y;

    errormsg(MAPDEBUG1,"DrawPiece: Finished");
    return MAPERR_Ok;
}

/*
**  Draw the frame around the selected map piece.
**/
static gulong DrawTFrame( MD *md, COORD newpiece, BOOL state)
{
    COORD ppos, len;
    static COORD opos;
    gulong fw, fh;

    errormsg(MAPDEBUG1,"DrawTFrame: Entered: nx=%u, ny=%u", newpiece.x, 
	     newpiece.y);

    if (!md->md_Frame)
        return(MAPERR_NoFrame);

    fw = md->child.style->klass->xthickness + 1 + md->md_FrameSpace;
    fh = md->child.style->klass->ythickness + 1 + md->md_FrameSpace;
#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"DrawTFrame: fw=%u, fh=%u", fw, fh);
#endif
    len.x = md->md_PWidth + 2*fw;
    len.y = md->md_PLength + 2*fh;
#if DEBUGLEV > 3
    errormsg(MAPDEBUG4,"DrawTFrame: len.x=%u, len.y=%u", len.x, len.y);
#endif
    ppos.x = (newpiece.x/len.x) * len.x;
    ppos.y = (newpiece.y/len.y) * len.y;
#if DEBUGLEV > 3
    errormsg(MAPDEBUG3,"DrawTFrame: ppos.x=%u, ppos.y=%u", ppos.x, ppos.y);
#endif
    if ( md->md_FrameToggle.l == newpiece.l &&
         md->md_FrameToggle.x == ppos.x &&
         md->md_FrameToggle.y == ppos.y )
    {
        if (state==FALSE)
        {
            opos = ppos;
        }
        else
        {
            ppos = opos;
        }
    }
    else
    {
        if (state==FALSE)
        {
            ppos = opos;
        }
        else
        {
            opos = ppos;
        }
    }
    /*
    **  Render the "frame" so it looks like a toggled widget.
    **/
    gdk_draw_rectangle(md->child.window, md->child.style->black_gc,
		       FALSE, ppos.x, ppos.y,
		       md->md_PWidth, md->md_PLength);

    gtk_paint_shadow (md->child.style, md->child.window,
		      GTK_STATE_NORMAL, GTK_SHADOW_IN, NULL, &md->child,
		      "text", ppos.x, ppos.y,
		      md->md_PWidth, md->md_PLength);

    errormsg(MAPDEBUG1,"DrawTFrame: Finished");
    return MAPERR_Ok;
}

/*
**  Set attributes.
**/
static void MapEditClassSet( GtkObject *obj, GtkArg *arg, guint arg_id )
{
    MD              *md = ( MD * ) GTK_MAPEDIT_SELECT( obj );
    struct MapPiece *npiece;
    struct MCMap    *amap=NULL;

    errormsg(MAPDEBUG3,"MapEditClassSet: Entered");

    switch( arg->type )
    {

      /*
      ** map piece change?
      */
      case MAPEDIT_CurrPiece:

        /*
        ** Did it really change ?
	*/
        npiece=(struct MapPiece *) GTK_VALUE_POINTER(*arg);
        if ( npiece && npiece->mp_PixbufNumber != md->md_CurrPiece.mp_PixbufNumber) {
            /*
            ** Yes. Show it and notify
            ** the change
            */
#if DEBUGLEV > 2
	    errormsg(MAPDEBUG3,"MapEditClassSet: CurrPiece=%x",npiece);
#endif
            if (npiece) md->md_CurrPiece=*npiece;
            NotifyAttrChange( obj, "mapedit_currpiece" );
        }

      case MAPEDIT_Grid:

	/*
	** Grid change (i.e. on or off)?
	*/
	{
	  BOOL nGrid;

	  if ( (nGrid=GTK_VALUE_BOOL(*arg)) != md->md_Grid ) 
	  {
#if DEBUGLEV > 2
	    errormsg(MAPDEBUG3,"MapEditClassSet: Grid=%u",nGrid);
#endif
            md->md_Grid = nGrid;
	    gtk_widget_queue_draw (GTK_WIDGET (md));
            NotifyAttrChange( obj, "mapedit_grid" );
	  }
	}

      case MAPEDIT_GridPen:

	/*
	** GridPen value changed?
	** Changes the color of the grid
	*/
	{
	  guchar nGridPen;

	  if ( (nGridPen=GTK_VALUE_UCHAR(*arg)) != md->md_GridPen ) 
	  {
#if DEBUGLEV > 2
	    errormsg(MAPDEBUG3,"MapEditClassSet: GridPen=%u",nGridPen);
#endif
            md->md_GridPen = nGridPen;
	    gtk_widget_queue_draw (GTK_WIDGET (md));
            NotifyAttrChange( obj, "mapedit_gridpen" );
	  }
	}

      case MAPEDIT_Map:

	/*
	** MAP changed?
	*/

        amap=(struct MCMap *) GTK_VALUE_POINTER(*arg);
        if (!amap) return; //( rc | MAPERR_NoMap );
#if DEBUGLEV > 2
	errormsg(MAPDEBUG3,"MapEditClassSet: amap=%x",amap);
#endif

        /*
        ** Backup old map for immediate Undo
        */
        if (!md->md_UndoBuffer) md->md_UndoBuffer=g_malloc0(sizeof(struct MCMap));
        if (!md->md_UndoBuffer) return;// ( rc | MAPERR_AllocFail );
        amap->mm_Copy = md->md_UndoBuffer->mm_Copy = md->md_Map->mm_Copy;

	errormsg(MAPDEBUG1,"MapEditClassSet: Backuped old map");

        /*
        ** Copy the new map into our map
        */
        if (md->md_Map->mm_Copy)
        {
            struct MCMap *tmap = md->md_UndoBuffer;
            MakeMap((md->md_Map->mm_Copy|COPYLAYER),tmap,md,md->md_Map);
            tmap = md->md_Map;
            MakeMap((md->md_Map->mm_Copy|COPYLAYER),tmap,md,amap);
        }
        else
        {
            md->md_UndoBuffer = md->md_Map;
            md->md_Map = amap;
        }
	gtk_widget_queue_draw (GTK_WIDGET (md));
        NotifyAttrChange( obj, "mapedit_map" );

      case MAPEDIT_SelectX:

	/*
	** Selected X pos changed?
	*/
	{
	  guint nSelectX;

	  if (!amap) return; //( rc | MAPERR_NoMap );

	  if ( (nSelectX=GTK_VALUE_UINT(*arg)) != md->md_Select.x ) 
	  {
#if DEBUGLEV > 2
	      errormsg(MAPDEBUG3,"MapEditClassSet: select.x=%u",nSelectX);
#endif
	      md->md_Select.x = nSelectX;
	      if (md->md_Select.x > md->md_Map->mm_MapSize.x) 
	        md->md_Select.x = md->md_Map->mm_MapSize.x;
	      gtk_widget_queue_draw (GTK_WIDGET (md));
	      NotifyAttrChange( obj, "mapedit_selectx" );
	  }
	}

      case MAPEDIT_SelectY:

	/*
	** Selected Y pos changed?
	*/
	{
	  guint nSelectY;

	  if (!amap) return;//( rc | MAPERR_NoMap );
	  if ( (nSelectY=GTK_VALUE_UINT(*arg)) != md->md_Select.y )
	  {
#if DEBUGLEV > 2
	      errormsg(MAPDEBUG3,"MapEditClassSet: select.y=%u",nSelectY);
#endif
	      md->md_Select.y = nSelectY;
	      if (md->md_Select.y > md->md_Map->mm_MapSize.y) 
		  md->md_Select.y = md->md_Map->mm_MapSize.y;
	      gtk_widget_queue_draw (GTK_WIDGET (md));
	      NotifyAttrChange( obj, "mapedit_selecty" );
	  }
	}

      case MAPEDIT_SelectL:
	/*
	** Selected Layer changed?
	*/
	{
	  guchar nSelectL;
	  if (!amap) return;//( rc | MAPERR_NoMap );

	  if ( (nSelectL=GTK_VALUE_UCHAR(*arg)) != md->md_Select.l) 
	  {
#if DEBUGLEV > 2
	      errormsg(MAPDEBUG3,"MapEditClassSet: select.l=%u",nSelectL);
#endif
	      md->md_Select.l = nSelectL;
	      if (md->md_Select.l > md->md_Map->mm_MapSize.l) 
		  md->md_Select.l = md->md_Map->mm_MapSize.l;
	      gtk_widget_queue_draw (GTK_WIDGET (md));
	      NotifyAttrChange( obj, "mapedit_selectl" );
	  }
	}
    }
    errormsg(MAPDEBUG1,"MapEditClassSet: Finished succesfully");
}

/*
**  Let's go active :) (mouse button pressed)
**/
static gint MapEditClassGoActive( GtkWidget *widget, GdkEventButton *event )
{
    MD          *md;
    gint         l, t;
    guint        w, h, fw, fh;
    COORD        newpiece;
    static BOOL oldtoggle = FALSE;

    errormsg(MAPDEBUG1,"MapEditClassGoActive: Entered");

    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_MAPEDIT (widget), FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

    md = ( MD * ) GTK_MAPEDIT_SELECT( widget );
    if (!md->md_Map) return FALSE;

#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"MapEditClassSet: md=%x",md);
#endif

    // Grab Focus if necessary
    if (!GTK_WIDGET_HAS_FOCUS (widget))
        gtk_widget_grab_focus (widget);

    w = widget->allocation.width;
    h = widget->allocation.height;

    /* Left mouse button pressed ? (we don't count the number pressed) */
    if (event->button == 1)
    {
        // All further mouse events are handled now by this class
        gtk_grab_add (widget);

        /*
	**  Get the coordinates
	**/
        l = event->x;
	t = event->y;
#if DEBUGLEV > 2
	errormsg(MAPDEBUG3,"MapEditClassGoActive: l=%u, t=%u, h=%u, w=%u",l,t,
		 w,h);
#endif

	fw = 2 * (md->child.style->klass->xthickness + 1 + md->md_FrameSpace);
	fh = 2* (md->child.style->klass->ythickness + 1 + md->md_FrameSpace);

	/*
	**  Are we really hit?
	**/
	if ( l >= 0 && t >= 0 && l < w && t < h )
	    if ( l <= (md->md_Map->mm_MapSize.x*(md->md_PWidth+fw)) && 
		      (md->md_Map->mm_MapSize.y*(md->md_PLength+fh)) )
	    {
		newpiece.x=l;
		newpiece.y=t;
		newpiece.l=md->md_Select.l;

		/*
		** Check, if we are Edit-Object
		*/
		if (md->md_GetPieces == TRUE)
		{
		    gulong fret = 0;
		    DrawTFrame( md, newpiece, TRUE );
		    if (oldtoggle)
		    {
			fret = DrawTFrame( md, newpiece, FALSE );
			oldtoggle = FALSE;
		    }
#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"MapEditClassGoActive: oldtoggle=%u, fret=%u",
			 oldtoggle,fret);
#endif
		    if (!fret)
		      oldtoggle = TRUE;
		    return FALSE;
		}

		/*
		**  Save actual Picture Area when going
		**  active. This way we can reset
		**  the initial buffer when the
		**  gadget activity is aborted by
		**  the user or intuition.
		**/
		if (md->md_InitialBuffer)
		  RemoveMap(md->md_InitialBuffer,md);
		md->md_InitialBuffer=g_malloc0(sizeof(struct MCMap));
		if (!md->md_InitialBuffer) return FALSE;//( rc|MAPERR_AllocFail );
		md->md_InitialBuffer->mm_Copy = md->md_Map->mm_Copy;
		if (md->md_Map->mm_Copy)
		{
		    MakeMap(md->md_Map->mm_Copy,
			    md->md_InitialBuffer,md,md->md_Map);
		}

		/*
		** Draw the new map piece or update the frame of it
		*/
		DrawPiece( md, &widget->allocation, newpiece );
		NotifyAttrChange( GTK_OBJECT(widget), "mapedit_map" );
	    }
    }
    // Check for right button (abort)
    else if (event->button == 2) 
    {
        /* Left mouse button wasn't pressed before */
        if(!gtk_grab_get_current)
	    return FALSE;
	
	// No longer look for mouse events
	gtk_grab_remove (widget);

	if (!md->md_GetPieces)
	{
	    /* Reset old area */
	  if (md->md_InitialBuffer)
	  {
	      MakeMap(md->md_Map->mm_Copy,md->md_Map,md,md->md_InitialBuffer);
	      RemoveMap(md->md_InitialBuffer,md);
	  }
	}
	else
	{
	    /*
	    **  Are we still in the hit area?
	    **/
	    l = md->md_CurrPiece.mp_PBCoord.x;
	    t = md->md_CurrPiece.mp_PBCoord.y;
	    if ( l >= 0 && t >= 0 && l < w && t < h )
	    {
	        if ( l <= (md->md_Map->mm_MapSize.x*md->md_PWidth) && 
		          (md->md_Map->mm_MapSize.y*md->md_PLength) )
		{
		    COORD len;
		    gulong fw, fh;
		    //DoMethod( md->md_Frame, OM_GET, FRM_FrameWidth,  &fw );
		    //DoMethod( md->md_Frame, OM_GET, FRM_FrameHeight, &fh );
		    fw = md->child.style->klass->xthickness;
		    fh = md->child.style->klass->ythickness;
#if DEBUGLEV > 2
		    errormsg(MAPDEBUG3,"DrawTFrame: fw=%u, fh=%u",fw,fh);
#endif
		    fw+=1+md->md_FrameSpace;
		    fh+=1+md->md_FrameSpace;
		    len.x = md->md_PWidth + fw;
		    len.y = md->md_PLength + fh;
		    md->md_FrameToggle.x = (md->md_CurrPiece.mp_PBCoord.x/len.x) * len.x;
		    md->md_FrameToggle.y = (md->md_CurrPiece.mp_PBCoord.y/len.y) * len.y;
		    md->md_FrameToggle.l = md->md_CurrPiece.mp_PBCoord.l;
		}
		else
		{
		    DrawTFrame( md, md->md_CurrPiece.mp_PBCoord, TRUE );
		    DrawTFrame( md, md->md_CurrPiece.mp_PBCoord, FALSE );
		}
	    }
	    else
	    {
		DrawTFrame( md, md->md_CurrPiece.mp_PBCoord, TRUE );
		DrawTFrame( md, md->md_CurrPiece.mp_PBCoord, FALSE );
	    }
	}

    }

    errormsg(MAPDEBUG1,"MapEditClassGoActive: Finished");

    return FALSE;
}

/*
**  Go inactive.
**/
static gint MapEditClassGoInactive( GtkWidget *widget, GdkEventButton *event)
{
    MD          *md;

    errormsg(MAPDEBUG1,"MapEditClassGoInActive: Entered");

    /*
    **  Reset initial map?
    **/

    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_MAPEDIT (widget), FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

    md = ( MD * ) GTK_MAPEDIT_SELECT( widget );

    if (!md->md_Map) return FALSE;

    // Only continue when mouse events are grabbed
    // else the draw event was aborted with right mouse button
    if(!gtk_grab_get_current)
        return FALSE;

    gtk_grab_remove (widget);

    if (md->md_GetPieces == TRUE)
    {
    // Fetch new mappiece under the mouse cursor
	md->md_CurrPiece.mp_PBCoord.x = md->md_AllPieces[md->md_CurrPiece.mp_PixbufNumber].mp_PBCoord.x;
	md->md_CurrPiece.mp_PBCoord.y = md->md_AllPieces[md->md_CurrPiece.mp_PixbufNumber].mp_PBCoord.y;
	NotifyAttrChange( GTK_OBJECT(widget), "map_currpiece" );
	return FALSE;
    }

    /* Save old area for undo */
    if (md->md_InitialBuffer)
    {
	if (!md->md_UndoBuffer) 
	    md->md_UndoBuffer=g_malloc0(sizeof(struct MCMap));
	if (!md->md_UndoBuffer) return FALSE;//( MAPERR_AllocFail );
	md->md_UndoBuffer->mm_Copy = md->md_Map->mm_Copy;
	MakeMap(md->md_Map->mm_Copy,md->md_UndoBuffer,md,md->md_InitialBuffer);
    }
    NotifyAttrChange( GTK_OBJECT(widget), "mapedit_map" );

    errormsg(MAPDEBUG1,"MapEditClassGoActive: Finished");

    return FALSE;
}

// Move around in the widget
static gint MapEditClassNotifyMove(GtkWidget *widget, GdkEventMotion *event)
{
    MD          *md;
    gint         x,y,mask;
    GdkModifierType mods;

    errormsg(MAPDEBUG1,"MapEditClassNotifyMove: Entered");

    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_MAPEDIT (widget), FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

    md = ( MD * ) GTK_MAPEDIT_SELECT( widget );

    if (!md->md_Map) return FALSE;

    x = event->x;
    y = event->y;
    mask = event->state;

    if (event->is_hint || (widget->window != event->window))
    {
        gdk_window_get_pointer (widget->window, &x, &y, &mods);
    }
    // ToDo: Scroll the map mapedit widget when right or bottom is reached

    errormsg(MAPDEBUG1,"MapEditClassNotifyMove: Finished");

    return FALSE;
}

/*
**  Notification about size changes
**/
static void MapEditClassNotifyDimensions( GtkWidget *widget, 
				    GtkAllocation *allocation )
{
    //MD          *md = ( MD * ) GTK_MAPEDIT_SELECT( widget );

    errormsg(MAPDEBUG1,"MapEditNotifyDimensions: Entered");

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_MAPEDIT (widget));
    g_return_if_fail (allocation != NULL);

#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"MapEditClassNotifyDimensions: allocation->x=%u, "
	     "allocation->y=%u, allocation->width=%u, allocation->height=%u",
	     allocation->x,allocation->y,allocation->width,allocation->height);
#endif

    widget->allocation = *allocation;

    if (GTK_WIDGET_REALIZED (widget))
    {
	gdk_window_move_resize (widget->window,
				allocation->x, allocation->y,
				allocation->width, 
				allocation->height);

	// Render with new size
	//RenderPixbuf(md, allocation);
    }

    errormsg(MAPDEBUG1,"MapEditNotifyDimensions: Finished");
}

static void MapEditAdjustmentChanged( GtkAdjustment *adjustment, 
				      gpointer data)
{
    MD *md;
    errormsg(MAPDEBUG1,"MapEditAdjustmentChanged: Entered");

    g_return_if_fail (adjustment != NULL);
    g_return_if_fail (data != NULL);
    g_return_if_fail (GTK_IS_MAPEDIT (data));
    
    md = GTK_MAPEDIT_SELECT (data);
    // Anything todo here ?

    errormsg(MAPDEBUG1,"MapEditAdjustmentChanged: Finished");
}

/*
 * Display another part of the map
 */
static void MapEditAdjustmentValueChanged( GtkAdjustment *adjustment, 
					   gpointer data)
{
    MD *md;

    errormsg(MAPDEBUG1,"MapEditAdjustmentValueChanged: Entered");

    g_return_if_fail (adjustment != NULL);
    g_return_if_fail (data != NULL);
    g_return_if_fail (GTK_IS_MAPEDIT (data));
    
    md = GTK_MAPEDIT_SELECT (data);

    if (GTK_WIDGET_VISIBLE (&md->child))
    {
        GtkAllocation myalloc;
	guint fw, fh;

	myalloc.x = 0;
	myalloc.y = 0;

	if (md->hadjustment->lower != (md->hadjustment->upper -
				       md->hadjustment->page_size))
	    myalloc.x = md->hadjustment->lower - md->hadjustment->value;

	if (md->vadjustment->lower != (md->vadjustment->upper -
				       md->vadjustment->page_size))
	    myalloc.y = md->vadjustment->lower - md->vadjustment->value;

	fw = md->child.style->klass->xthickness + 1 + md->md_FrameSpace;
	fh = md->child.style->klass->ythickness + 1 + md->md_FrameSpace;
	md->md_Select.x = (-1 * myalloc.x) / (md->md_PWidth + (md->md_Frame?2*fw:0));
	md->md_Select.y = (-1 * myalloc.y) / (md->md_PLength + (md->md_Frame?2*fh:0));

	if (GTK_WIDGET_REALIZED (md))
	{
#if DEBUGLEV > 4
	    errormsg(MAPDEBUG5,"myalloc.x=%u, myalloc.y=%u, sel.x=%u, sel.y=%u"
		     ", pw=%u, ph=%u",(-1 * myalloc.x),(-1 * myalloc.y), 
		     md->md_Select.x, md->md_Select.y, 
		     (md->md_PWidth + (md->md_Frame?2*fw:0)),
		     (md->md_PLength + (md->md_Frame?2*fh:0)));
#endif
	    gdk_window_move (md->child.window,
			     myalloc.x,
			     myalloc.y);
	}
    }

    errormsg(MAPDEBUG1,"MapEditAdjustmentValueChanged: Finished");
}


// Methods defined by MapEditClass (use ExecMethod())


/*
** Get map piece number from Map position
** Do not forget to test if we are out of range
*/
static gulong GetXYPiece( GtkObject *obj, struct mapSelect * msl )
{
    MD          *md = ( MD * ) GTK_MAPEDIT_SELECT( obj );
    struct MCMap *amap = md->md_Map;
    gulong t;

    if (!amap) return MAPERR_NoMap;
    if (msl->LayerNum && !md->md_Map->mm_MapSize.l)
        return MAPERR_NoLayer;
    if (msl->LayerNum > md->md_Map->mm_MapSize.l)
        return MAPERR_UnknownLayer;
    else
    {
        int lnum = 1;
        while (lnum!=msl->LayerNum)
        {
            if (amap->mm_NextLayer)
            {
                amap = amap->mm_NextLayer;
            }
            else
            {
                return(MAPERR_UnknownLayer);
            }
            lnum++;
        }
    }
    if (msl->Xpos > md->md_Map->mm_MapSize.x || msl->Ypos > md->md_Map->mm_MapSize.y)
        return MAPERR_OutOfBounds;
    amap->mm_Rows = (void *) amap->mm_Columns[msl->Ypos];
    t= (gulong) amap->mm_Rows + amap->mm_Rows->mp_Size * msl->Xpos;
    amap->mm_Rows = (struct MapPiece *)t;
    msl->PieceNum = amap->mm_Rows;
    return MAPERR_Ok;
}

/*
** Clear the map
** We want to be blank :-)
*/
static void ClearMap( GtkObject *obj )
{
    MD          *md = ( MD * ) GTK_MAPEDIT_SELECT( obj );
    struct MCMap *dmap = md->md_Map;
    gulong i,j;

    if (!dmap) return;
    do
    {
        for (i=0; i< md->md_Map->mm_MapSize.y;i++)  // All columns have to be cleared
        {
            dmap->mm_Rows = (void *) dmap->mm_Columns[i];
            for (j=0; j< md->md_Map->mm_MapSize.x;j++) // and all Rows need to be cleared
            {
                dmap->mm_Rows[j].mp_PixbufNumber = md->md_Default->mp_PixbufNumber;
                dmap->mm_Rows[j].mp_PBCoord.x = md->md_AllPieces[dmap->mm_Rows[j].mp_PixbufNumber].mp_PBCoord.x;
                dmap->mm_Rows[j].mp_PBCoord.y = md->md_AllPieces[dmap->mm_Rows[j].mp_PixbufNumber].mp_PBCoord.y;
            }
        }
        dmap = dmap->mm_NextLayer;
    } while (dmap);

    gtk_widget_queue_draw (GTK_WIDGET (md));
}

/*
** Set map piece number to Map position
** Do not forget to test if we are out of range
*/
static gulong SetMapPiece( GtkObject *obj, struct mapSelect * msl )
{
    MD          *md = ( MD * ) GTK_MAPEDIT_SELECT( obj );
    struct MCMap *amap = md->md_Map;

    if (!amap) return MAPERR_NoMap;
    if (msl->LayerNum && !md->md_Map->mm_MapSize.l)
        return MAPERR_NoLayer;
    if (msl->LayerNum > md->md_Map->mm_MapSize.l)
        return MAPERR_UnknownLayer;
    else
    {
        int lnum = 1;
        while (lnum!=msl->LayerNum)
        {
            if (amap->mm_NextLayer)
            {
                amap = amap->mm_NextLayer;
            }
            else
            {
                return(MAPERR_UnknownLayer);
            }
            lnum++;
        }
    }
    if (msl->Xpos > md->md_Map->mm_MapSize.x || msl->Ypos > md->md_Map->mm_MapSize.y)
        return MAPERR_OutOfBounds;
    amap->mm_Rows = (void *) amap->mm_Columns[md->md_Select.y];
    amap->mm_Rows[md->md_Select.x].mp_PixbufNumber = msl->PieceNum->mp_PixbufNumber;
    amap->mm_Rows[md->md_Select.x].mp_PBCoord.x = md->md_AllPieces[amap->mm_Rows[md->md_Select.x].mp_PixbufNumber].mp_PBCoord.x;
    amap->mm_Rows[md->md_Select.x].mp_PBCoord.y = md->md_AllPieces[amap->mm_Rows[md->md_Select.x].mp_PixbufNumber].mp_PBCoord.y;

    gtk_widget_queue_draw (GTK_WIDGET (md));
    return MAPERR_Ok;
}

/*
** Fill map section with one map piece
** Do not forget to test if we are or get out of range (partial fill)
*/
static gulong FillMap( GtkWidget *widget, struct mapSection * mst )
{
    MD          *md = ( MD * ) GTK_MAPEDIT_SELECT( widget );
    struct MCMap *amap = md->md_Map;
    gulong i,j, sw, sl;

    if (!amap) return MAPERR_NoMap;
    if (mst->LayerNum && !md->md_Map->mm_MapSize.l)
        return MAPERR_NoLayer;
    if (mst->LayerNum > md->md_Map->mm_MapSize.l)
        return MAPERR_UnknownLayer;
    else
    {
        int lnum = 1;
        while (lnum!=mst->LayerNum)
        {
            if (amap->mm_NextLayer)
            {
                amap = amap->mm_NextLayer;
            }
            else
            {
                return(MAPERR_UnknownLayer);
            }
            lnum++;
        }
    }
    if ((mst->Xpos+mst->Width) > md->md_Map->mm_MapSize.x ||
        (mst->Ypos+mst->Length) > md->md_Map->mm_MapSize.y)
        return MAPERR_OutOfBounds;
    sw = mst->Xpos + mst->Width;
    sl = mst->Ypos + mst->Length;
    for(i=mst->Ypos;i< sl;i++)
    {
        amap->mm_Rows = (void *) amap->mm_Columns[i];
        for (j=mst->Xpos;j<sl;j++)
        {
            amap->mm_Rows[j].mp_PixbufNumber = md->md_CurrPiece.mp_PixbufNumber;
            amap->mm_Rows[j].mp_PBCoord.x = md->md_AllPieces[md->md_CurrPiece.mp_PixbufNumber].mp_PBCoord.x;
            amap->mm_Rows[j].mp_PBCoord.y = md->md_AllPieces[md->md_CurrPiece.mp_PixbufNumber].mp_PBCoord.y;
        }
    }

    //DoMethod(obj, GM_RENDER, mst->mstInfo, rp, GREDRAW_REDRAW);
    RenderPixbuf(md, &widget->allocation);
    return MAPERR_Ok;
}

/*
** Delete the map
** Attention: Next redraw will fail, if you do not set up a new map
** Why waste memory ;-)
*/
static void DeleteMap( GtkObject *obj )
{
    MD          *md = ( MD * ) GTK_MAPEDIT_SELECT( obj );

    if (!md->md_Map) return; // Map already deleted ?
    if (md->md_Map->mm_Copy & COPYMAP)
        RemoveMap(md->md_Map,md);
    else
        md->md_Map = NULL;
    
}

/*
** Gets the pixbuf coordinates from the given pixbuf number
**
*/
static void GetPixbufCoord( GtkObject *obj, struct PBCoord *bc )
{
    MD          *md = ( MD * ) GTK_MAPEDIT_SELECT( obj );
    bc->Coord.x = md->md_AllPieces[bc->PixbufNumber].mp_PBCoord.x;
    bc->Coord.y = md->md_AllPieces[bc->PixbufNumber].mp_PBCoord.y;
}

/*
** Copy UndoBuffer to Map Object Area
** Map Object Area -> UndoBuffer currently not implemented.
*/
static void RestoreOldPicture( GtkObject *obj )
{
    MD          *md = ( MD * ) GTK_MAPEDIT_SELECT( obj );
    if (!md->md_Map) return;
    if (md->md_UndoBuffer)
    {
        MakeMap(md->md_Map->mm_Copy|COPYLAYER,md->md_Map,md,md->md_UndoBuffer);
        RemoveMap(md->md_UndoBuffer,md);
    }
    // Undo which can be undone again:
    // Search the last layer, copy the current map there
    // Copy the Undobuffer to the MapEdit Map -> contains to many layers
    // Go to the last layer (md->md_Map->mm_MapSize.l) and set next layer to 0
    // At the same time go to the last UndoBuffer layer and set next layer to Undobuffer
}

gulong ExecMethod(guint method, GtkObject *obj, GtkArg *arg, guint arg_id)
{
    switch(method)
    {
        case MAP_GetXYPiece:
	    return GetXYPiece( obj, 
			       (struct mapSelect *) GTK_VALUE_POINTER(*arg));
	    break;
        case MAP_Clear:
	    ClearMap( obj );
	    return MAPERR_Ok;
	    break;
        case MAP_Set:
	    return SetMapPiece( obj,
				(struct mapSelect *) GTK_VALUE_POINTER(*arg));
	    break;
        case MAP_Fill:
	    return FillMap ( GTK_WIDGET(obj),
			     (struct mapSection *) GTK_VALUE_POINTER(*arg));
	    break;
        case MAP_Undo:
	    RestoreOldPicture( obj );
	    return MAPERR_Ok;
	    break;
        case MAP_BitmapCoord:
	    GetPixbufCoord( obj,(struct PBCoord *) GTK_VALUE_POINTER(*arg));
	    return MAPERR_Ok;	    
	    break;
        case MAP_Delete:
	    DeleteMap ( obj );
	    return MAPERR_Ok;	    
	    break;
        default:
	    return MAPERR_Unknown;
    }
}

/*
**  Initialize the class.
**  Creates all signals which can be sent to an object of this class.
**  Connects all methods (own ones, object, wiget) to the object.
**/
static void InitMapEditClass( GtkMapEditClass *class )
{
  // Casting classes, no real inheritance possible in C
    GtkObjectClass  *ocl = (GtkObjectClass *) class;
    GtkWidgetClass  *wcl = (GtkWidgetClass *) class;

    errormsg(MAPDEBUG1,"InitMapEditClass: Entered");

    mapedit_signals[MAPEDIT_CURRPIECE_SIGNAL] = 
      gtk_signal_new ("mapedit_currpiece", GTK_RUN_LAST, ocl->type,
		      GTK_SIGNAL_OFFSET (GtkMapEditClass, currpiece),
		      gtk_signal_default_marshaller, GTK_TYPE_ULONG, 0);
    mapedit_signals[MAPEDIT_GRID_SIGNAL] = 
      gtk_signal_new ("mapedit_grid", GTK_RUN_LAST, ocl->type,
		      GTK_SIGNAL_OFFSET (GtkMapEditClass, grid),
		      gtk_signal_default_marshaller, GTK_TYPE_ULONG, 0);
    mapedit_signals[MAPEDIT_GRIDPEN_SIGNAL] = 
      gtk_signal_new ("mapedit_gridpen", GTK_RUN_LAST, ocl->type,
		      GTK_SIGNAL_OFFSET (GtkMapEditClass, gridpen),
		      gtk_signal_default_marshaller, GTK_TYPE_ULONG, 0);
    mapedit_signals[MAPEDIT_MAP_SIGNAL] = 
      gtk_signal_new ("mapedit_map", GTK_RUN_LAST, ocl->type,
		      GTK_SIGNAL_OFFSET (GtkMapEditClass, map),
		      gtk_signal_default_marshaller, GTK_TYPE_ULONG, 0);
    mapedit_signals[MAPEDIT_SELECTX_SIGNAL] = 
      gtk_signal_new ("mapedit_selectx", GTK_RUN_LAST, ocl->type,
		      GTK_SIGNAL_OFFSET (GtkMapEditClass, selectx),
		      gtk_signal_default_marshaller, GTK_TYPE_ULONG, 0);
    mapedit_signals[MAPEDIT_SELECTY_SIGNAL] = 
      gtk_signal_new ("mapedit_selecty", GTK_RUN_LAST, ocl->type,
		      GTK_SIGNAL_OFFSET (GtkMapEditClass, selecty),
		      gtk_signal_default_marshaller, GTK_TYPE_ULONG, 0);
    mapedit_signals[MAPEDIT_SELECTL_SIGNAL] = 
      gtk_signal_new ("mapedit_selectl", GTK_RUN_LAST, ocl->type,
		      GTK_SIGNAL_OFFSET (GtkMapEditClass, selectl),
		      gtk_signal_default_marshaller, GTK_TYPE_ULONG, 0);
    gtk_object_class_add_signals (ocl, mapedit_signals, LAST_SIGNAL);

    // Replace destroy function
    ocl->destroy = MapEditClassDispose;
    ocl->set_arg = MapEditClassSet;
    ocl->get_arg = MapEditClassGet;

    mapedit_parent_class = gtk_type_class(GTK_TYPE_WIDGET);

    // No default handlers (at least in this function)
    class->currpiece = NULL;
    class->grid = NULL;
    class->gridpen = NULL;
    class->map = NULL;
    class->selectx = NULL;
    class->selecty = NULL;
    class->selectl = NULL;

    // Events which have to be known by our class
    wcl->realize = MapEditClassRealize;
    wcl->expose_event = MapEditClassRender;
    wcl->draw = MapEditClassDraw;
    wcl->size_request = MapEditClassSizeReq; // Replaces MAPEDIT_BoxWidth ?
    wcl->size_allocate = MapEditClassNotifyDimensions;
    wcl->button_press_event = MapEditClassGoActive;
    wcl->button_release_event = MapEditClassGoInactive;
    wcl->motion_notify_event = MapEditClassNotifyMove;
    //wcl->enter_notify_event = MapEditClassEntered;
    //wcl->leave_notify_event = MapEditClassLeft;
    /*wcl->set_scroll_adjustments_signal = 
      gtk_signal_new("set_scroll_adjustment", GTK_RUN_LAST, ocl->type,
		     GTK_SIGNAL_OFFSET(GtkMapEditClass, setscrolladjustments),
		     gtk_marshal_NONE__POINTER_POINTER, GTK_TYPE_NONE, 2,
		     GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT); */
    /*
    **  Setup dispatcher.
    **/
    //cl->cl_Dispatcher.h_Entry = ( HOOKFUNC )MapEditClassDispatch;

    errormsg(MAPDEBUG1,"InitMapEditClass: Finished");
}

/*
** MapEditClass_get_type - Create or return the unique ID of this object
*/
guint  MapEditClassGetType(void)
{
    static guint me_type = 0;

    //errormsg(MAPDEBUG1,"MapEditClassGetType: Entered");

    if (!me_type)
    {
        static const GtkTypeInfo me_info =
	{
	    "MapEditClass",
	    sizeof (MD),
	    sizeof (GtkMapEditClass),
	    (GtkClassInitFunc) InitMapEditClass,
	    (GtkObjectInitFunc) MapEditInit,
	    NULL, NULL, (GtkClassInitFunc) NULL
	};
	me_type = gtk_type_unique (gtk_widget_get_type(), &me_info);
    }

    //errormsg(MAPDEBUG1,"MapEditClassGetType: Finished");
    return me_type;
}
