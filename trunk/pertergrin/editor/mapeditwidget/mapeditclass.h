#ifndef MAPEDITCLASS_H
#define MAPEDITCLASS_H
/*
**  $VER: mapeditclass.h 0.0.2 (10.04.2001)
**  C Header file for the Map Edit Widget Class.
**
**  (C) 2000/2001 by Reinhard Katzmann
**/

/* Make C++ happy */
#ifdef __cplusplus
       extern "C" {
#endif /* __cplusplus */

typedef int             BOOL;       /* 0=false, <>0 true */

/* Prototypes */
//BOOL FreeMapEditClass( Class * );

/*
**  Koordinate pair structure (including layer)
*/
typedef struct COORD {
        guint x;
        guint y;
        guchar l;
} COORD;

typedef struct PBCoord {
    gulong    MethodID;   /* MAP_BitmapCoord */
    guint    PixbufNumber;
    COORD    Coord;
} PBCoord;

/*
**  Map piece structure (including User expandibility and size)
*/
typedef struct MapPiece
{
    gulong          mp_Number;        /* Number of map piece */
    guint           mp_PixbufNumber;  /* The pixbuf number of this map piece */
    COORD           mp_Coordinates;   /* Coordinates of the map piece inside map */
    COORD           mp_PBCoord;       /* Coordinates of the map piece inside Pixbuf */
    size_t          mp_Size;          /* Size of the MapPiece structure */
    void           *mp_User;          /* User extensions to this structure */
} sMapPiece;

/*
**  Map structure (including User expandibility and size)
*/
typedef struct MCMap
{
    gulong           *mm_Columns;       /* Columns store all the rows */
    struct MapPiece  *mm_Rows;          /* Rows store the map pieces itself */
    COORD             mm_MapSize;       /* Size of the map (width, height, layers) */
    size_t            mm_Size;          /* Size of the MCMap structure */
    void             *mm_User;          /* User extensions to this structure */
    BOOL              mm_Copy;          /* Is this a map copy or not ? */
    struct MCMap     *mm_NextLayer;     /* Pointer to next layer if any */
} sMCMap;

/*
**  MapEditClass object instance data.
**/
typedef struct _GtkMapData {
    GtkWidget        *child;           /* object data */
    struct MCMap     *md_Map;          /* map data (2D, BMap-like w. layers) */
    GdkPixbuf        *md_MapPieces;    /* PixBuf containing all map pieces   */
    guint             md_PWidth;       /* Width of one map piece             */
    guint             md_PLength;      /* Length of one map piece            */
    BOOL              md_Copy;         /* Copy BitMap or not                 */
    struct MapPiece   md_CurrPiece;    /* Currently selected map piece       */
    struct MapPiece  *md_Default;      /* Default map piece (for clearing)   */
    struct MapPiece  *md_AllPieces;    /* For building map pieces for maps   */
    BOOL              md_Frame;        /* Frame (shadow) around map pieces   */
    guchar            md_FrameSpace;   /* Space between framed map pieces    */
    COORD             md_FrameToggle;  /* Position of toggled frame          */
    BOOL              md_GetPieces;    /* Edit mode/map piece get mode       */
    BOOL              md_Grid;         /* If TRUE Grid around map pieces     */
    guchar            md_GridPen;      /* Selected GridPen.                  */
    COORD             md_Select;       /* Selected map position              */
//  guint             md_BoxWidth;     /* current width of the object        */
//  guint             md_BoxHeight;    /* current height of the object       */
    guchar            md_ScaleWidth;   /* show width percentage of the map   */
    guchar            md_ScaleHeight;  /* show height percentage of the map  */
    struct MCMap     *md_UndoBuffer;   /* UndoBuffer, for application        */
    struct MCMap     *md_InitialBuffer;/* Initial Buffer (Rendering)         */
} MD;

/*
** MapEditClass Widget methods
**/
typedef struct _GtkMapEditClass
{
  GtkWidgetClass parent_class;

  void (* currpiece)  (MD *md);
  void (* grid) (MD *md);
  void (* gridpen)  (MD *md);
  void (* map)    (MD *md);
  void (* selectx)    (MD *md);
  void (* selecty)    (MD *md);
  void (* selectl)    (MD *md);
} GtkMapEditClass;

/* GtkArg Arguments The commentary show when the args can be used */
/* I: new, S: set_arg, G: get_arg, S: set_arg, N: Notify, U: Update */
enum {
  MAPEDIT_NULL,
  MAPEDIT_MapPieces,   /* I---- */
  MAPEDIT_MapWidth,    /* I-G-- */
  MAPEDIT_MapLength,   /* I-G-- */
  MAPEDIT_MapLayer,    /* I-G-- */
  MAPEDIT_NoCopy,      /* I---- */
  MAPEDIT_GetPieces,   /* I---- */
  MAPEDIT_CurrPiece,   /* ISGNU */
  MAPEDIT_Grid,        /* ISGNU */
  MAPEDIT_GridPen,     /* ISGNU */
  MAPEDIT_Map,         /* ISGNU */
  MAPEDIT_Default,     /* I---- */
  MAPEDIT_Frame,       /* I---- */
  MAPEDIT_FrameSpace,  /* I---- */
  MAPEDIT_SelectX,     /* ISGNU */
  MAPEDIT_SelectY,     /* ISGNU */
  MAPEDIT_SelectL,     /* ISGNU */
  MAPEDIT_PWidth,      /* I---- */
  MAPEDIT_PLength,     /* I---- */
  // Mabye those two get replaced by size methods
  MAPEDIT_BoxWidth,    /* ---N- */
  MAPEDIT_BoxHeight,   /* ---N- */
  MAPEDIT_ScaleWidth,  /* I---- */
  MAPEDIT_ScaleHeight, /* I---- */
};

#define GTK_TYPE_MAPEDIT         (MapEditClassGetType())
#define GTK_MAPEDIT_SELECT(obj)  (GTK_CHECK_CAST((obj), GTK_TYPE_MAPEDIT, MD)) 
#define GTK_IS_MAPEDIT(obj)      (GTK_CHECK_TYPE((obj), GTK_TYPE_MAPEDIT))
enum mapmsg_types { MAPINFO, MAPERROR, MAPWARNING, MAPDEBUG1, MAPDEBUG2,
		    MAPDEBUG3, MAPDEBUG4, MAPDEBUG5, MAPMSG, MAPDEBUG6, 
		    MAPPARANOIA };

void errormsg(short type, char *errmsg, ...);
GtkWidget *MapEditClassNew(GtkArg *args, guint num_args);

/* Methods */

#define MAP_GetXYPiece    "mapgetxypiece"
#define MAP_Clear         "mapclear"
#define MAP_Set           "mapset"
#define MAP_Fill          "mapfill"
#define MAP_Undo          "mapundo"
#define MAP_BitmapCoord   "mapbitmapcoord"
#define MAP_Delete        "mapdelete"

struct mapSelect {
    gulong   MethodID;   /* MAGM_GetXYPiece, MAP_Set */
    struct GadgetInfo *mslInfo;  /* ignored by MAGM_GetXYPiece */
    guint    Xpos;
    guint    Ypos;
    guint    LayerNum;
    struct MapPiece *PieceNum;
};

typedef struct mapSection {
    gulong   MethodID;   /* MAP_Fill */
    struct WidgetInfo *mstInfo;
    guint    Xpos;
    guint    Ypos;
    guint    LayerNum;
    guint    Width;
    guint    Length;
} mapSection;

struct WidgetMsg
{
    gulong   MethodID;
    struct WidgetInfo *wInfo;
};

typedef struct WidgetMsg *WidgetMsg;

/* OM_SET & MapEdit methods may return these: */

#define MAPERR_Ok            (0)    /* No problems */
#define MAPERR_NoChange      (1 << 9)  /* No Undo has been done */
#define MAPERR_NoAttr        (1 << 10) /* You did not specify a necessary attribute */
#define MAPERR_AllocFail     (1 << 11) /* AllocVec() failure */
#define MAPERR_NoMap         (1 << 12) /* Your Map was NULL */
#define MAPERR_Fatal         (1 << 13) /* A fatal failure (not closer specified) */
#define MAPERR_NoLayer       (1 << 14) /* Map has no additional layers */
#define MAPERR_OutOfBounds   (1 << 15) /* wrong Map position/coordinates */
#define MAPERR_UnknownLayer  (1 << 16) /* Layer out of range */
#define MAPERR_NoFrame       (1 << 17) /* No frame object */

/* TAG_USER+0x70000 through TAG_USER+0x700040 reserved. */

#ifdef __cplusplus
       }
#endif /* __cplusplus */
#endif
