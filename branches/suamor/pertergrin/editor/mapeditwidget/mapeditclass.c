/*
 * MapEditClass.c (C) 2000/2001 by Reinhard Katzmann. All rights reserved
 *
 * GTK (Gimp Tool Kit) Map Edit gadget class including possibility to select
 * mappieces so you can also create a "GetPiece" gadget.
 * It is FREEWARE. Usage is restricted. Please refer to the included
 * MapEditClass.readme for more information. By using this class you
 * automatically agree to the License in the MapEditClass.readme file.
 * (Short note: The GNU Library General Public License Version 2 applies
 *  to this code as well for freeware programmers).
 */

#include <sys/types.h>
// #include <sys/commsize.h>
#include <memory.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mikmod.h> // only included for types, to lazy to extract them ;-)

#include <glib.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gdk/gdktypes.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <widgets/mapeditclass.h> // Attention: Version numbering is here!

extern char *getLogfile();
guint  MapEditClassGetType(void);
GtkWidget *MapEditClassNew(GtkArg **args, guint num_args);
static void MapEditClassSet( GtkObject *obj, GtkArg *arg );
static void MapEditClassRealize (GtkWidget *widget);
static gint MapEditClassRender( GtkWidget *widget, GdkEventExpose *event );
static void MapEditClassSizeReq (GtkWidget *widget,
				 GtkRequisition *requisition);
static void MapEditClassDimensions( GtkWidget *widget, 
				    GtkAllocation *allocation );
static void InitMapEditClass( GtkMapEditClass *class );
static void MapEditInit( MD *md );

GtkWidgetClass *mapedit_parent_class = NULL;

/* Debug options */
#define NOERROR 0
#define NOWARNING 0
#define NOINFO 0
#define DEBUG 1     // Set to 1 to enable debugging
#define DEBUGLEV 5       /* 1: Informations only,
			  * 2: Add Error messages
			  * 3: Add Warning messages
			  * 4: Add entry and exit messages
			  * 5: Add important variables output
			  * 6: Add most variables output
			  * 7: Add several function calls outside
			  * 8: Add all variables output
			  * 9: Add all function calls outside
			  * 10: Add all kind of silly output
			  */
enum mapmsg_types { MAPINFO, MAPERROR, MAPWARNING, MAPDEBUG1, MAPDEBUG2,
		    MAPDEBUG3, MAPDEBUG4, MAPDEBUG5, MAPDEBUG6, MAPPARANOIA };
/*
**  Simple type-cast.
**/
#define GAD(x)      (( struct Widget * )x)

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
  char prmsg[1024]="";
  FILE *fh;

  switch(type)
  {
    case MAPERROR:
      if (!NOERROR) sprintf(prmsg,"Error: ");
      else return;
      break;
    case MAPWARNING:
      if (!NOWARNING) sprintf(prmsg,"Warning: ");
      else return;
      break;
    case MAPINFO:
      if (!NOINFO) sprintf(prmsg,"Info: ");
      else return;
      break;
    case MAPDEBUG1:
      if (DEBUGLEV>0) sprintf(prmsg,"Debug1: ");
      else return;
      break;
    case MAPDEBUG2:
      if (DEBUGLEV>1) sprintf(prmsg,"Debug2: ");
      else return;
      break;
    case MAPDEBUG3:
      if (DEBUGLEV>2) sprintf(prmsg,"Debug3: ");
      else return;
      break;
    case MAPDEBUG4:
      if (DEBUGLEV>3) sprintf(prmsg,"Debug4: ");
      else return;
      break;
    case MAPDEBUG5:
      if (DEBUGLEV>4) sprintf(prmsg,"Debug5: ");
      else return;
      break;
    case MAPDEBUG6:
      if (DEBUGLEV>5) sprintf(prmsg,"Debug6: ");
      else return;
      break;
    case MAPPARANOIA:
      if (DEBUGLEV>6) sprintf(prmsg,"Paranoia: ");
      else return;
      break;
  }
  strcat(prmsg,errmsg);
  strcat(prmsg,"\n");
  if (prmsg[0])
  {
    va_list argp;

    fh=fopen(getLogfile(),"a+");
    va_start(argp,errmsg);
    vfprintf(fh,prmsg,argp);
    va_end(argp);
    fclose(fh);
  }
#endif
}

/*
** MakeMap  - Allocates a new map and optionally makes a copy of another map
**
** UBYTE mkcopy        - If set, makes a copy of smap (and optionally its layers)
**                       when creating a new dmap.
** struct MCMap *dmap  - the map to be created.
** MD           *md    - pointer to object instance data.
** struct MCMap *smap  - the (optional) map to be copied.
**
** Returns: TRUE on success, FALSE otherwise
*/
static BOOL MakeMap(UBYTE mkcopy, struct MCMap *dmap, MD *md, 
		    struct MCMap *smap)
{
    UBYTE lnum = 0;
    ULONG i,j;
    errormsg(MAPDEBUG1,"MakeMap start");

#if DEBUGLEV > 1
    errormsg(MAPDEBUG2,"MakeMap: mkcopy=%d dmap=%x, md=%x, smap=%",mkcopy,
	      dmap,md,smap);
#endif    
    do // Iterate through all layers
    {
        if (smap) // Default (i.e. smap!=NULL): Use default values
            dmap->mm_MapSize = smap->mm_MapSize;
        else
            dmap->mm_MapSize = md->md_Map->mm_MapSize; // Set default value
#if DEBUGLEV > 5
	errormsg(MAPDEBUG6,"MakeMap: Allocating mm_Columns");
#endif
        dmap->mm_Columns = g_new0(ULONG,md->md_Map->mm_MapSize.y);
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
                dmap->mm_Rows = g_malloc0(md->md_Default->mp_Size * md->md_Map->mm_MapSize.x );
#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"MakeMap: dmap=%x, md=%x, smap=%x", dmap,
			 md, smap);
#endif
#if DEBUGLEV > 1
		errormsg(MAPDEBUG2,"MakeMap: dmap->mm_Rows=%x",dmap->mm_Rows);
#endif
                if (dmap->mm_Rows)
                {
                    // Store allocated row in current column
                    dmap->mm_Columns[i] = (ULONG)dmap->mm_Rows;
                    for (j=0; j< md->md_Map->mm_MapSize.x;j++) // Finally put MapPieces in rows
                    {
                        dmap->mm_Rows[j].mp_Coordinates.x = j * md->md_PWidth;
                        dmap->mm_Rows[j].mp_Coordinates.y = i * md->md_PLength;
                        dmap->mm_Rows[j].mp_Coordinates.l = lnum;
#if DEBUGLEV > 4
			errormsg(MAPDEBUG5,"MakeMap: smap=%x", smap);
#endif
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
	errormsg(MAPDEBUG5,"MakeMap: mkcopy=%d, COPYLAYER=%d", mkcopy, 
		 COPYLAYER);
#endif
        if ( (mkcopy & COPYLAYER) && lnum < md->md_Map->mm_MapSize.l )
        {
#if DEBUGLEV > 4
	    errormsg(MAPDEBUG5,"MakeMap: mkcopy=%d, COPYMAP=%d", mkcopy, 
		     COPYMAP);
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
	    errormsg(MAPDEBUG5,"MakeMap: mkcopy=%d, COPYMAP=%d", mkcopy, 
		     COPYMAP);
#endif
            if (! (mkcopy & COPYMAP))
            /* Allocate own layer only if smap has no next layer or is not valid */
            {
#if DEBUGLEV > 4
	        errormsg(MAPDEBUG5,"MakeMap: smap=%x, smap->NextLayer=%x",
			 smap,smap->mm_NextLayer);
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
#if DEBUGLEV > 4
		errormsg(MAPDEBUG5,"MakeMap: smap=%x",smap);
#endif
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
    errormsg(MAPDEBUG2,"lnum=%d, md->md_Map->mm_MapSize.l=%d", lnum, 
	     md->md_Map->mm_MapSize.l);
#endif
    if (lnum > md->md_Map->mm_MapSize.l) // This should not happen
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
    UBYTE lnum = 0;
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
	errormsg(MAPDEBUG5,"RemoveMap: x-Size: %d, y-Size: %d, Layer-Size: %d", 
		 amap->mm_MapSize.x, amap->mm_MapSize.y, amap->mm_MapSize.l);
#endif
        if (amap && amap->mm_Columns)
        {
            int i;
            for (i=0; i< md->md_Map->mm_MapSize.y;i++) // All columns have to be freed
            {
#if DEBUGLEV > 4
	        errormsg(MAPDEBUG5,"RemoveMap: amap=%x, md=%x, "
			 "amap->mm_Columns[%d]=%x", amap, md, i, 
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
	errormsg(MAPDEBUG5,"RemoveMap: amap=%x, tmap=%x, lnum=%d", amap, md, 
		 tmap, lnum);
#endif
        g_free(amap);        // Free this layer
        amap=tmap;            // Set new current layer
        lnum++;
    } while (amap);
    md->md_Map=NULL;
    errormsg(MAPDEBUG1,"RemoveMap succesfully finished");
    return TRUE;
}

/*
**  Initialize a new mapedit object.
**/
static void MapEditInit(MD *md)
{
  /*
  **  Preset the data to 0. Don't
  **  know if this is necessary.
  **/
  errormsg(MAPDEBUG1,"MapEditInit: Setting up md structure");

  bzero(( char * )md, sizeof( MD ));

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
  md->md_Default                    = NULL;
  md->md_Select.x = md->md_Select.y = md->md_Select.l = 0;
  md->md_FrameToggle                = md->md_Select;
  errormsg(MAPDEBUG1,"MapEditInit: Finished setting up md structure");
}

/*
**  Create a new mapedit object.
**/
GtkWidget *MapEditClassNew(GtkArg **args, guint num_args)
{
    MD              *md =  gtk_type_new GTK_TYPE_MAPEDIT;
    GdkPixbuf       *bm;
    struct MCMap    *amap=NULL;
    ULONG           arg,i, mapx=0, mapy=0, mapl = 0;
    BOOL            mkcopy = FALSE;

    errormsg(MAPDEBUG1,"MapEditClassNew: Creating New Widget");

    /*
    **  Setup the instance data.
    **/
    for (arg = 0 ; arg < num_args ; arg++)
    {
        // Failure when one argument is incorrectly initialised
        if (args[arg]==NULL) return NULL;

        // Check all the arguments
#if DEBUGLEV > 2
	errormsg(MAPDEBUG3,"args[%d}=%x",arg,args[arg]);
#endif
	switch ( args[arg]->type )
	{
	    case MAPEDIT_NoCopy:
	        errormsg(MAPINFO,"MapEditClassNew: Nocopy is set");
#if DEBUGLEV > 1
	        errormsg(MAPDEBUG2,"MapEditClassNew: md->md_Copy=%d",
			 md->md_Copy);
#endif
		if (GTK_VALUE_BOOL(*args[arg]) && md->md_Copy)
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
		md->md_Copy=!GTK_VALUE_BOOL(*args[arg]);
		break;

	    case MAPEDIT_MapPieces:
		errormsg(MAPINFO,"MapEditClassNew: MapPieces is set");
		bm = (GdkPixbuf *) GTK_VALUE_POINTER(*args[arg]);
		if (md->md_Copy) {
		    if (md->md_MapPieces) gdk_pixbuf_unref(md->md_MapPieces);
		    md->md_MapPieces = NULL;
		    md->md_MapPieces=gdk_pixbuf_copy(bm);
#if DEBUGLEV > 2
		    errormsg(MAPDEBUG3,"MapPieces=%x,bm=%x",
			     md->md_MapPieces,bm);
#endif
		}
		else md->md_MapPieces=gdk_pixbuf_ref(bm);
		break;

	    case MAPEDIT_PWidth:
                errormsg(MAPINFO,"MapEditClassNew: PWidth is set");
		md->md_PWidth = GTK_VALUE_UINT(*args[arg]);
		break;

            case MAPEDIT_PLength:
                errormsg(MAPINFO,"MapEditClassNew: PLength is set");
		md->md_PLength = GTK_VALUE_UINT(*args[arg]);
		break;

	    case MAPEDIT_Map:
                errormsg(MAPINFO,"MapEditClassNew: Map is set");
		amap = (struct MCMap *) GTK_VALUE_POINTER(*args[arg]);
		if (amap->mm_Copy) mkcopy = TRUE;
		break;

	    case MAPEDIT_MapWidth:
                errormsg(MAPINFO,"MapEditClassNew: MapWidth is set");
		mapx = GTK_VALUE_UINT(*args[arg]);
		break;

	    case MAPEDIT_MapLength:
                errormsg(MAPINFO,"MapEditClassNew: MapLength is set");
		mapy = GTK_VALUE_UINT(*args[arg]);
		break;

	    case MAPEDIT_MapLayer:
                errormsg(MAPINFO,"MapEditClassNew: MapLayer is set");
		mapl = GTK_VALUE_UCHAR(*args[arg]);
		break;

	    case MAPEDIT_GetPieces:
                errormsg(MAPINFO,"MapEditClassNew: GetPieces is set");
		md->md_GetPieces = GTK_VALUE_BOOL(*args[arg]);
		break;

	    case MAPEDIT_Frame:
                errormsg(MAPINFO,"MapEditClassNew: Frame is set");
		md->md_Frame = GTK_VALUE_BOOL(*args[arg]);
		break;

	    case MAPEDIT_FrameSpace:
                errormsg(MAPINFO,"MapEditClassNew: FrameSpace is set");
		md->md_FrameSpace = GTK_VALUE_UCHAR(*args[arg]);
		break;

	    case MAPEDIT_CurrPiece:
                errormsg(MAPINFO,"MapEditClassNew: CurrPiece is set");
		md->md_CurrPiece.mp_Number = ((struct MapPiece *)GTK_VALUE_POINTER(*args[arg]))->mp_Number;
		md->md_CurrPiece.mp_PixbufNumber = ((struct MapPiece *)GTK_VALUE_POINTER(*args[arg]))->mp_PixbufNumber;
		md->md_CurrPiece.mp_PBCoord = ((struct MapPiece *)GTK_VALUE_POINTER(*args[arg]))->mp_PBCoord;
		md->md_CurrPiece.mp_Size = ((struct MapPiece *)GTK_VALUE_POINTER(*args[arg]))->mp_Size;
		md->md_CurrPiece.mp_User = ((struct MapPiece *)GTK_VALUE_POINTER(*args[arg]))->mp_User;
                break;

	    case MAPEDIT_Default:
                errormsg(MAPINFO,"MapEditClassNew: Default is set");
		md->md_Default = (struct MapPiece *) GTK_VALUE_POINTER(*args[arg]);
		break;

	    case MAPEDIT_Grid:
                errormsg(MAPINFO,"MapEditClassNew: Grid is set");
		md->md_Grid = GTK_VALUE_BOOL(*args[arg]);
		break;

	    case MAPEDIT_GridPen:
                errormsg(MAPINFO,"MapEditClassNew: GridPen is set");
		md->md_GridPen = GTK_VALUE_UCHAR(*args[arg]);
		break;

	    case MAPEDIT_SelectX:
	        errormsg(MAPINFO,"MapEditClassNew: SelectX is set");
		md->md_Select.x = GTK_VALUE_UINT(*args[arg]);
		break;

	    case MAPEDIT_SelectY:
	        errormsg(MAPINFO,"MapEditClassNew: SelectY is set");
		md->md_Select.y = GTK_VALUE_UINT(*args[arg]);
		break;

	    case MAPEDIT_SelectL:
	        errormsg(MAPINFO,"MapEditClassNew: SelectL is set");
		md->md_Select.l = GTK_VALUE_UCHAR(*args[arg]);
		break;

	    case MAPEDIT_ScaleWidth:
	        errormsg(MAPINFO,"MapEditClassNew: ScaleWidth is set");
		md->md_ScaleWidth = GTK_VALUE_UCHAR(*args[arg]);
		break;

	    case MAPEDIT_ScaleHeight:
	        errormsg(MAPINFO,"MapEditClassNew: ScaleHeight is set");
		md->md_ScaleHeight = GTK_VALUE_UCHAR(*args[arg]);
		break;
	}
    }

    /*
    **  Test if we have all attributes
    **  and allocate the map
    */
#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"MapPieces=%x,md->md_PWidth=%d, md->md_PLength=%d,"
	     "md->md_Default=%d",md->md_MapPieces,md->md_PWidth,
	     md->md_PLength, md->md_Default);
#endif
    if (md->md_MapPieces && md->md_PWidth && md->md_PLength && md->md_Default)
    {
#if DEBUGLEV > 2
        errormsg(MAPDEBUG3,"mapx=%d,mapy=%d,MINROW=%d,MINCOL=%d",
		 mapx, mapy, MINROW, MINCOL);
#endif
        if (mapx >= MINROW && mapy >= MINCOL)
	{
#if DEBUGLEV > 2
	    errormsg(MAPDEBUG3,"amap=%x, mkcopy=%d",amap,mkcopy);
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
	        UWORD wtn, htn;
		BOOL ret=FALSE;

		// Calculate pixbuf coordinates
                wtn = gdk_pixbuf_get_width(md->md_MapPieces) / md->md_PWidth;
	        htn = gdk_pixbuf_get_height(md->md_MapPieces) / md->md_PLength;
#if DEBUGLEV > 1
		errormsg(MAPDEBUG2,"wtn=%d,htn=%d",wtn, htn);
#endif

		if (md->md_Default->mp_Number==0)
		{ // Calculate number of map pieces
		    md->md_Default->mp_Number = wtn *  htn;
		}

		if(md->md_Default->mp_Size==0)
		    md->md_Default->mp_Size = sizeof(struct MapPiece);

#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"md->md_Default->mp_Number=%d,"
			 "md->md_Default->mp_PixbufNumber=%d",
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
		      md->md_AllPieces[i].mp_PBCoord.x = i%wtn;
		      md->md_AllPieces[i].mp_PBCoord.y = i/wtn;
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
			  errormsg(MAPDEBUG3,"kkcopy=%d,tmap=%x,md=%x,amap=%x",
				   mkcopy, tmap, md, amap);
#endif
			    ret=MakeMap(mkcopy,tmap,md,amap);
			}
		    }
		    else
		    {
		        do // Iterate through all layers
			{
			    ULONG i,j;
			    for (i=0; i< md->md_Map->mm_MapSize.y;i++) // All columns have to be filled
			    {
			        amap->mm_Rows = (void *) amap->mm_Columns[i];
#if DEBUGLEV > 2
				errormsg(MAPDEBUG3,"amap->mm_Rows=%x,"
					 "amap->mm_Columns[%d]=%x",
					 amap->mm_Rows,i,amap->mm_Columns[i]);
#endif
				for (j=0; j< md->md_Map->mm_MapSize.x;j++) // Calculate MapPieces coordinates
				{
				    amap->mm_Rows[j].mp_PBCoord.x = md->md_AllPieces[amap->mm_Rows[j].mp_PixbufNumber].mp_PBCoord.x;
				    amap->mm_Rows[j].mp_PBCoord.y = md->md_AllPieces[amap->mm_Rows[j].mp_PixbufNumber].mp_PBCoord.y;
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
			md->md_FrameToggle.l = 0;
			return GTK_WIDGET ( md );
    
		    }
		}
	    }
	}
    }
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
static void MapEditClassGet( GtkObject *obj, GtkArg *arg )
{
    MD          *md = ( MD * ) GTK_MAPEDIT_SELECT( obj );
    ULONG       i;
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
	  errormsg(MAPDEBUG2,"MapEditClassGet: Map=%x, mm_MapSize.x=%d, md=%x",
		   md->md_Map, md->md_Map->mm_MapSize.x, md);
#endif
            GTK_VALUE_UINT(*arg) = md->md_Map->mm_MapSize.x;
            break;

        case MAPEDIT_MapLength:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: Map=%x, mm_MapSize.y=%d, md=%x",
		   md->md_Map, md->md_Map->mm_MapSize.y, md);
#endif
            GTK_VALUE_UINT(*arg) = md->md_Map->mm_MapSize.y;
            break;

        case MAPEDIT_MapLayer:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: Map=%x, mm_MapSize.y=%d, md=%x",
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
	  errormsg(MAPDEBUG2,"MapEditClassGet: md->md_Grid=%d, md=%x",
		   md->md_Grid, md);
#endif
            GTK_VALUE_BOOL(*arg) = md->md_Grid;
            break;

        case MAPEDIT_GridPen:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: md->md_GridPen=%d, md=%x",
		   md->md_GridPen, md);
#endif
            GTK_VALUE_UCHAR(*arg) = md->md_GridPen;
            break;

        case MAPEDIT_SelectX:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: md->md_Select.x=%d, md=%x",
		   md->md_Select.x, md);
#endif
            GTK_VALUE_UINT(*arg) = md->md_Select.x;
            break;

        case MAPEDIT_SelectY:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: md->md_Select.y=%d, md=%x",
		   md->md_Select.y, md);
#endif
            GTK_VALUE_UINT(*arg) = md->md_Select.y;
            break;

        case MAPEDIT_SelectL:
#if DEBUGLEV > 1
	  errormsg(MAPDEBUG2,"MapEditClassGet: md->md_Select.l=%d, md=%x",
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
    GdkWindowAttr attributes;
    gint attributes_mask;

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
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.window_type = GDK_WINDOW_CHILD;
    // Add events which interests us
    attributes.event_mask = gtk_widget_get_events (widget) | 
      GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | 
      GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
    widget->window = gdk_window_new (widget->parent->window, &attributes, 
				     attributes_mask);
    widget->style = gtk_style_attach (widget->style, widget->window);
    gdk_window_set_user_data (widget->window, widget);
    gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);

    errormsg(MAPDEBUG1,"MapEditClassRealize finished");
}

static void MapEditClassSizeReq (GtkWidget *widget,
				 GtkRequisition *requisition)
{
    MD *md = GTK_MAPEDIT_SELECT(widget);
    ULONG width=md->md_PWidth,height=md->md_PLength;

    errormsg(MAPDEBUG1,"MapEditClassSizeReq entered");

#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"widget=%x, width=%d, height=%d",widget,width,height);
#endif
    width=(width*md->md_ScaleWidth)/100;
    height=(height*md->md_ScaleHeight)/100;
#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"Now width=%d, height=%d",width,height);
#endif
    if ((width+MINCOL) > MINCOL) width-=MINCOL;
    if ((height+MINROW) > MINROW) height-=MINROW;

    requisition->width = MINROW + width;
    requisition->height = MINCOL + height;

    errormsg(MAPDEBUG1,"MapEditClassSizeReq finished");
}

/*
**  Render the Pixbuf of our object
**/
static BOOL RenderPixbuf( MD *md, GtkAllocation *area)
{
    UWORD       colsize, rowsize;
    UWORD       left, top, xpos, ypos, selx, sely; //, color=0;
    ULONG       cx, cy, i, j, fw = 0, fh = 0, ysize;
    /* char zeile[256]; */
    GdkPixbuf *tmppb; /* temporary Pixbuf for scaling */
    static struct MCMap *amap = NULL;


    errormsg(MAPDEBUG1,"RenderPixbuf: Entered");

#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"RenderPixbuf: md=%x, area=%x",md,area);
#endif
    /*
    **  Get initial left and top offset.
    **/
    if (!amap) amap = md->md_Map;
    if (!amap) return FALSE;
    left = area->x + 1;
    top  = area->y + 1;

#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"RenderPixbuf: amap=%x, left=%d, top=%d",amap,left,top);
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
	    fw = md->child->style->klass->xthickness;
	    fh = md->child->style->klass->ythickness;
#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"RenderPixbuf: fw=%d, fh=%d",fw,fh);
#endif
            fw+=1+md->md_FrameSpace;
            fh+=1+md->md_FrameSpace;
            cx=(area->width/(md->md_PWidth+fw))*(md->md_PWidth+fw);
            cy=(area->height/(md->md_PLength+fh))*(md->md_PLength+fh);
#if DEBUGLEV > 3
    errormsg(MAPDEBUG4,"Renderpixbuf: Now fw=%d, fh=%d, cx=%d, cy=%d",
	     fw, fh, cx, cy);
#endif
        }
        else
        {
            cx = (area->width/md->md_PWidth)*md->md_PWidth;
            cy = (area->height/md->md_PLength)*md->md_PLength;
#if DEBUGLEV > 3
    errormsg(MAPDEBUG4,"Renderpixbuf: cx=%d, cy=%d", cx, cy);
#endif
        }
    }
    else
    {
        cx = (area->width/(md->md_PWidth+(md->md_Grid ? 1 : 0)))*(md->md_PWidth+(md->md_Grid ? 1 : 0));
        cy = (area->height/(md->md_PLength+(md->md_Grid ? 1 : 0)))*(md->md_PLength+(md->md_Grid ? 1 : 0));
#if DEBUGLEV > 3
    errormsg(MAPDEBUG4,"Renderpixbuf: cx=%d, cy=%d", cx, cy);
#endif
    }

    /*
    **  Allocate temporary Pixbuf which will be blitted
    */
    tmppb=gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, cx, cy);

    if (!tmppb) return (FALSE);
#if DEBUGLEV > 3
    errormsg(MAPDEBUG4,"Renderpixbuf: tmppb=%x", tmppb);
#endif

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
            lnum = 0;
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
	errormsg(MAPDEBUG4,"Renderpixbuf: layer=%d, amap=%x", lnum, amap);
#endif
    }

    /*
    **  Calculate absolute selected map position
    */
    selx = md->md_Select.x * md->md_PLength;
    sely = md->md_Select.y * md->md_PWidth;
    ysize = md->md_Default->mp_Size * md->md_Select.y;
#if DEBUGLEV > 3
    errormsg(MAPDEBUG4,"Renderpixbuf: selx=%d, sely=%d, ysize=%d", selx, sely,
	     ysize);
#endif

    /*
    **  Calculate width and height of the section to be blitted
    **  including relative selected map position
    */
    rowsize = cx/md->md_PWidth + md->md_Select.x;
    if (rowsize > amap->mm_MapSize.y) rowsize = amap->mm_MapSize.y;
    colsize = cy/md->md_PLength + md->md_Select.y;
    if (colsize > amap->mm_MapSize.x) colsize = amap->mm_MapSize.x;
#if DEBUGLEV > 3
    errormsg(MAPDEBUG4,"Renderpixbuf: rowsize=%d, colsize=%d", rowsize, 
	     colsize);
#endif

    /*
    **  Copy map pieces into temporary Pixbuf based on the section of the Map
    **  TODO: First find the correct layer (Maybe only in MAPEDIT_SelectL !!)
    */
    for (i=md->md_Select.y;i<colsize;i++)
    {
        amap->mm_Rows = (void *) (amap->mm_Columns[i] + ysize);
        for(j=md->md_Select.x;j<rowsize;j++)
        {
            xpos = amap->mm_Rows[j].mp_Coordinates.x - selx;
            ypos = amap->mm_Rows[j].mp_Coordinates.y - sely;

            if (md->md_GetPieces)
            {
                if (md->md_Frame)
                {
		    gdk_draw_rectangle(md->child->window, 
				       md->child->style->black_gc,
				       FALSE, left+xpos, top+ypos,
				       md->md_PWidth, md->md_PLength);

		    gtk_paint_shadow (md->child->style, md->child->window,
				      GTK_STATE_NORMAL, GTK_SHADOW_IN,
				      NULL, md->child, "text",
				      left+xpos, top+ypos,
				      md->md_PWidth, md->md_PLength);
                    xpos+=fw;
                    ypos+=fh;
                }

            }
            else
            {
                xpos += (md->md_Grid ? 1 : 0); /* Destination position x */
                ypos += (md->md_Grid ? 1 : 0); /* Destination position y */

                /* Draw Grid if wished */
                if (md->md_Grid)
                {
                    if (!xpos%cx) 
		        gdk_draw_rectangle(md->child->window, 
					   md->child->style->black_gc,
					   TRUE, left, ypos,
					   left+area->width-1, ypos);

                    if (!ypos%cy) 
		        gdk_draw_rectangle(md->child->window, 
					   md->child->style->black_gc,
					   TRUE, xpos, top,
					   xpos, top+area->height-1);

#if DEBUGLEV > 3
		    errormsg(MAPDEBUG4,"Renderpixbuf: Grid at %d,%d-%d,%d and"
			     " %d,%d-%d,%d", left, ypos, left+area->width-1, 
			     ypos, xpos, top, xpos, top+area->height-1);
#endif
                }
            }
	    gdk_pixbuf_copy_area(md->md_MapPieces,
				 md->md_Map->mm_Rows[j].mp_PBCoord.x,
				 amap->mm_Rows[j].mp_PBCoord.y, md->md_PWidth,
				 md->md_PLength, tmppb, xpos+fw, ypos+fh);
        }
    }
    gdk_pixbuf_render_to_drawable(tmppb, md->child->window,
				  md->child->style->fg_gc[GTK_STATE_NORMAL],
				  0, 0, left, top, cx, cy, 
				  GDK_RGB_DITHER_NORMAL,
				  left, top);

    // Draw the toggle gadget
    if ( md->md_Frame && md->md_FrameToggle.l == md->md_Select.l &&
        (md->md_FrameToggle.x-selx) < cx &&
        (md->md_FrameToggle.y-sely) < cy )
    {
        /*
        **  Render the "frame" so it looks like a toggled widget.
        **/
        gdk_draw_rectangle(md->child->window, md->child->style->black_gc,
			   FALSE, left+md->md_FrameToggle.x-selx, 
			   top+md->md_FrameToggle.y-sely,
			   md->md_PWidth, md->md_PLength);

	gtk_paint_shadow (md->child->style, md->child->window,
			  GTK_STATE_NORMAL, GTK_SHADOW_OUT, NULL, md->child,
			  "text", left+md->md_FrameToggle.x-selx,
			  top+md->md_FrameToggle.y-sely,
			  md->md_PWidth, md->md_PLength);
    }
    /*
    **  Free temporary Pixbuf
    */
    gdk_pixbuf_unref(tmppb);
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
    //ULONG           fw = 0, fh = 0;

    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_MAPEDIT (widget), FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

    if (md->child != widget)
    {
        errormsg(MAPINFO,"Expose: widget=%x, md->child=%x",widget,md->child);
        md->child=widget;
    }
    // Do not render more than once (but pretend you just had)
    if (event->count > 0)
        return FALSE;

    /*
    **  Render the pixel rectangles.
    **/
    if (!RenderPixbuf( md, &widget->allocation ))
        return FALSE; // Don't know what to return in case this fails

        return FALSE;
}

/*
**  Draw a piece with the currently selected map piece.
**/
static ULONG DrawPiece( MD *md, GtkAllocation *area, COORD newpiece)
{
    COORD ppos, len;
    struct MCMap *amap = md->md_Map;

    len.x = md->md_PWidth + md->md_Grid ? 1 : 0;
    len.y = md->md_PLength + md->md_Grid ? 1 : 0;
    ppos.x = (newpiece.x/len.x) * len.x;
    ppos.y = (newpiece.y/len.y) * len.y;

    if (!amap) return( MAPERR_NoMap );
    if (md->md_Map->mm_MapSize.l)
    {
        int lnum = 0;
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


    gdk_pixbuf_render_to_drawable(md->md_MapPieces, md->child->window,
				  md->child->style->fg_gc[GTK_STATE_NORMAL],
				  amap->mm_Rows[md->md_Select.x].mp_PBCoord.x,
				  amap->mm_Rows[md->md_Select.x].mp_PBCoord.y,
				  ppos.x, ppos.y, md->md_PWidth, 
				  md->md_PLength, GDK_RGB_DITHER_NORMAL,
				  amap->mm_Rows[md->md_Select.x].mp_PBCoord.x,
				  amap->mm_Rows[md->md_Select.x].mp_PBCoord.y);

    md->md_CurrPiece.mp_Coordinates.l = md->md_Select.l;
    md->md_CurrPiece.mp_Coordinates.x = amap->mm_Rows[md->md_Select.x].mp_Coordinates.x;
    md->md_CurrPiece.mp_Coordinates.y = amap->mm_Rows[md->md_Select.x].mp_Coordinates.y;
    return MAPERR_Ok;
}

/*
**  Draw a piece with the currently selected map piece.
**/
static ULONG DrawTFrame( MD *md, COORD newpiece, BOOL state)
{
    COORD ppos, len;
    static COORD opos;
    ULONG fw, fh;

    if (!md->md_Frame)
        return(MAPERR_NoFrame);
    //DoMethod( md->md_Frame, OM_GET, FRM_FrameWidth,  &fw );
    //DoMethod( md->md_Frame, OM_GET, FRM_FrameHeight, &fh );
    fw = md->child->style->klass->xthickness;
    fh = md->child->style->klass->ythickness;
#if DEBUGLEV > 2
    errormsg(MAPDEBUG3,"DrawTFrame: fw=%d, fh=%d",fw,fh);
#endif
    fw+=1+md->md_FrameSpace;
    fh+=1+md->md_FrameSpace;
    len.x = md->md_PWidth + fw;
    len.y = md->md_PLength + fh;
#if DEBUGLEV > 3
    errormsg(MAPDEBUG4,"DrawTFrame: Now fw=%d, fh=%d, len.x=%d, len.y=%d",
	     fw, fh, len.x, len.y);
#endif
    ppos.x = (newpiece.x/len.x) * len.x;
    ppos.y = (newpiece.y/len.y) * len.y;
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
    gdk_draw_rectangle(md->child->window, md->child->style->black_gc,
		       FALSE, ppos.x, ppos.y,
		       md->md_PWidth, md->md_PLength);

    gtk_paint_shadow (md->child->style, md->child->window,
		      GTK_STATE_NORMAL, GTK_SHADOW_OUT, NULL, md->child,
		      "text", ppos.x, ppos.y,
		      md->md_PWidth, md->md_PLength);

    return MAPERR_Ok;
}

/*
**  Set attributes.
**/
static void MapEditClassSet( GtkObject *obj, GtkArg *arg )
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
	    errormsg(MAPDEBUG3,"MapEditClassSet: Grid=%d",nGrid);
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
	  UBYTE nGridPen;

	  if ( (nGridPen=GTK_VALUE_UCHAR(*arg)) != md->md_GridPen ) 
	  {
#if DEBUGLEV > 2
	    errormsg(MAPDEBUG3,"MapEditClassSet: GridPen=%d",nGridPen);
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
	  UWORD nSelectX;

	  if (!amap) return; //( rc | MAPERR_NoMap );

	  if ( (nSelectX=GTK_VALUE_UINT(*arg)) != md->md_Select.x ) 
	  {
#if DEBUGLEV > 2
	      errormsg(MAPDEBUG3,"MapEditClassSet: select.x=%d",nSelectX);
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
	  UWORD nSelectY;

	  if (!amap) return;//( rc | MAPERR_NoMap );
	  if ( (nSelectY=GTK_VALUE_UINT(*arg)) != md->md_Select.y )
	  {
#if DEBUGLEV > 2
	      errormsg(MAPDEBUG3,"MapEditClassSet: select.y=%d",nSelectY);
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
	  UBYTE nSelectL;
	  if (!amap) return;//( rc | MAPERR_NoMap );

	  if ( (nSelectL=GTK_VALUE_UCHAR(*arg)) != md->md_Select.l) 
	  {
#if DEBUGLEV > 2
	      errormsg(MAPDEBUG3,"MapEditClassSet: select.l=%d",nSelectL);
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
    UWORD        w, h;
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

    // Left mouse button pressed ? (we don't count the number pressed)
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
	errormsg(MAPDEBUG3,"MapEditClassGoActive: l=%d, t=%d, h=%d, w=%d",l,t,
		 w,h);
#endif

	/*
	**  Are we really hit?
	**/
	if ( l >= 0 && t >= 0 && l < w && t < h )
	    if ( l <= (md->md_Map->mm_MapSize.x*md->md_PWidth) && 
		      (md->md_Map->mm_MapSize.y*md->md_PLength) )
	    {
		newpiece.x=l;
		newpiece.y=t;
		newpiece.l=md->md_Select.l;

		/*
		** Check, if we are Edit-Object
		*/
		if (md->md_GetPieces == TRUE)
		{
		    ULONG fret = 0;
		    DrawTFrame( md, newpiece, TRUE );
		    if (oldtoggle)
		    {
			fret = DrawTFrame( md, newpiece, FALSE );
			oldtoggle = FALSE;
		    }
#if DEBUGLEV > 2
		errormsg(MAPDEBUG3,"MapEditClassGoActive: oldtoggle=%d, fret=%d",
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
        // Left mouse button wasn't pressed before
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
		    ULONG fw, fh;
		    //DoMethod( md->md_Frame, OM_GET, FRM_FrameWidth,  &fw );
		    //DoMethod( md->md_Frame, OM_GET, FRM_FrameHeight, &fh );
		    fw = md->child->style->klass->xthickness;
		    fh = md->child->style->klass->ythickness;
#if DEBUGLEV > 2
		    errormsg(MAPDEBUG3,"DrawTFrame: fw=%d, fh=%d",fw,fh);
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
    return FALSE;
}

/*
**  Go inactive.
**/
static gint MapEditClassGoInactive( GtkWidget *widget, GdkEventButton *event)
{
    MD          *md;

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

    return FALSE;
}

// Move around in the widget
static gint MapEditClassNotifyMove(GtkWidget *widget, GdkEventMotion *event)
{
    MD          *md;
    gint         x,y,mask;
    GdkModifierType mods;

    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_MAPEDIT (widget), FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

    // Only continue when mouse events are grabbed
    if(!gtk_grab_get_current)
        return FALSE;

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

    return FALSE;
}

/*
**  Tell'm our minimum dimensions.
**/
static void MapEditClassDimensions( GtkWidget *widget, 
				    GtkAllocation *allocation )
{
    MD          *md = ( MD * ) GTK_MAPEDIT_SELECT( widget );
    ULONG       width=md->md_PWidth,height=md->md_PLength;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_MAPEDIT (widget));
    g_return_if_fail (allocation != NULL);

    widget->allocation = *allocation;

    if (GTK_WIDGET_REALIZED (widget))
    {
        width=(width*md->md_ScaleWidth)/100;
	height=(height*md->md_ScaleHeight)/100;

	if ((width+MINCOL) > MINCOL) width-=MINCOL;
	if ((height+MINROW) > MINROW) height-=MINROW;

	gdk_window_move_resize (widget->window,
				allocation->x, allocation->y,
				allocation->width /* + (MINROW + width) */, 
				allocation->height /* + (MINCOL + height) */);
    }

    //*( dim->grmd_MinSize.Width  ) += (MINROW + width);
    //*( dim->grmd_MinSize.Height ) += (MINCOL + height);
}

/*
** Get map piece number from Map position
** Do not forget to test if we are out of range
*/
static ULONG GetXYPiece( GtkObject *obj, struct mapSelect * msl )
{
    MD          *md = ( MD * ) GTK_MAPEDIT_SELECT( obj );
    struct MCMap *amap = md->md_Map;
    ULONG t;

    if (!amap) return MAPERR_NoMap;
    if (msl->LayerNum && !md->md_Map->mm_MapSize.l)
        return MAPERR_NoLayer;
    if (msl->LayerNum > md->md_Map->mm_MapSize.l)
        return MAPERR_UnknownLayer;
    else
    {
        int lnum = 0;
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
    t= (ULONG) amap->mm_Rows + amap->mm_Rows->mp_Size * msl->Xpos;
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
    ULONG i,j;

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
static ULONG SetMapPiece( GtkObject *obj, struct mapSelect * msl )
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
        int lnum = 0;
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
static ULONG FillMap( GtkObject *obj, struct mapSection * mst )
{
    MD          *md = ( MD * ) GTK_MAPEDIT_SELECT( obj );
    struct MCMap *amap = md->md_Map;
    ULONG i,j, sw, sl;

    if (!amap) return MAPERR_NoMap;
    if (mst->LayerNum && !md->md_Map->mm_MapSize.l)
        return MAPERR_NoLayer;
    if (mst->LayerNum > md->md_Map->mm_MapSize.l)
        return MAPERR_UnknownLayer;
    else
    {
        int lnum = 0;
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
    RenderPixbuf(md, &md->md_GraphBox);
    return MAPERR_Ok;
}

/*
** Delete the map
** Attention: Next redraw will fail, if you do not set up a new map
** Why waste memory ;-)
*/
static void DeleteMap( GtkObject *obj, WidgetMsg gmsg )
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
static void RestoreOldPicture( GtkObject *obj, WidgetMsg msg )
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
    // Shouldn't be necessary, see MapEditClassGetType()
    //ocl->set_arg = MapEditClassSet;
    //ocl->get_arg = MapEditClassGet;

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
    wcl->size_request = MapEditClassSizeReq; // Replaces MAPEDIT_BoxWidth ?
    wcl->size_allocate = MapEditClassDimensions;
    wcl->button_press_event = MapEditClassGoActive;
    wcl->button_release_event = MapEditClassGoInactive;
    wcl->motion_notify_event = MapEditClassNotifyMove;
    //wcl->enter_notify_event = MapEditClassGoActive;
    //wcl->leave_notify_event = MapEditClassGoInactive;
    /*
    **  Setup dispatcher.
    **/
    //cl->cl_Dispatcher.h_Entry = ( HOOKFUNC )MapEditClassDispatch;
}

/*
** MapEditClass_get_type - Create or return the unique ID of this object
*/
guint  MapEditClassGetType(void)
{
    static guint me_type = 0;

    if (!me_type)
    {
        GtkTypeInfo me_info =
	{
	    "MapEditClass",
	    sizeof (MD),
	    sizeof (GtkMapEditClass),
	    (GtkClassInitFunc) InitMapEditClass,
	    (GtkObjectInitFunc) MapEditInit,
	    (GtkArgSetFunc) MapEditClassGet,
	    (GtkArgGetFunc) MapEditClassSet
	};
	me_type = gtk_type_unique (gtk_widget_get_type(), &me_info);
    }
    return me_type;
}
