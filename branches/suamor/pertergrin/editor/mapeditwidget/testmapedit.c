/*
**  testmapedit.c (C) 2000/2001 Reinhard Katzmann. All rights reserved.
**
**  A program to test the mapeditclass widget. Like the mapeditclass
**  itself it is distributed under the GPL License. See the mapeditclass
**  widget source or documentation for more informations.
*/

#include <sys/types.h>
#include <memory.h>
#include <glib.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gdktypes.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <widgets/mapeditclass.h>

#define SELXSIZE 4
#define SELYSIZE 4
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

GtkMapEditClass *MapEditClass;
GtkObject TestMEdWindowObjs[WO_NUMGADS];
GdkWindow *window;
GdkPixbuf *mMapPieces = NULL;

UBYTE *WO_MXGAD1Labels[] =
{
    "MPG1",
    "MPG2",
    "MPG3",
    NULL
};

/*
 *      Map-lists.
 */
ULONG e2rx[] = { MAPEDIT_BoxWidth, PGA_Visible, TAG_END };
ULONG e2ry[] = { MAPEDIT_BoxHeight, PGA_Visible, TAG_END };

GdkObject       *WO_Window;
GdkObject       *filereq;
void *dtobject=NULL; /* Pointer to a datatypes object */

void CloseAll(void)
{
    char z[256];
    struct MCMap *SelectGroup = NULL;
    ULONG pt=0;

    if (dtobject) DisposeDTObject(dtobject);
    if (filereq) DisposeObject( filereq);
    if (TestMEdWindowObjs[WO_MPGRP1])
    {
        GetAttr(MAPEDIT_Map, (Object *)TestMEdWindowObjs[WO_MPGRP1], &pt);
        SelectGroup = (struct MCMap *)pt;
        sprintf(z,"CloseAll start: Map=%x, pt=%x, SelectGroup=%x\n", (Object *)TestMEdWindowObjs[WO_MPGRP1], pt, SelectGroup);
        KPrintF(z);
        if (SelectGroup)
        {
            do
            {
                struct MCMap *tmap=NULL;
                sprintf(z,"CloseAll do: SelectGroup=%x, SelectGroup->mm_Columns=%x\n", SelectGroup, SelectGroup->mm_Columns);
                KPrintF(z);
                if (SelectGroup->mm_Columns)
                {
                    int i;
                    for (i=0; i< SELYSIZE;i++) // All columns have to be freed
                    {
                        sprintf(z,"CloseAll for: SelectGroup=%x, SelectGroup->mm_Columns[%d]=%x\n", SelectGroup, i, SelectGroup->mm_Columns[i]);
                        KPrintF(z);
                        if (SelectGroup->mm_Columns[i])
                            FreeVec((APTR)SelectGroup->mm_Columns[i]);
                        SelectGroup->mm_Columns[i]=0;
                    }
                    FreeVec(SelectGroup->mm_Columns);
                    SelectGroup->mm_Columns=NULL;
                }
                sprintf(z,"CloseAll Layer: SelectGroup=%x, SelectGroup->mm_NextLayer=%x\n", SelectGroup, SelectGroup->mm_NextLayer);
                KPrintF(z);
                tmap = SelectGroup->mm_NextLayer; // Get pointer to next layer
                SelectGroup->mm_NextLayer = NULL; // Do not reference it again!
                FreeVec(SelectGroup);        // Free this layer
                SelectGroup=tmap;     // Set new current layer
            } while (SelectGroup);
        }
        DoGadgetMethod((struct Gadget *)TestMEdWindowObjs[WO_MPGRP1],window,NULL,MAP_Delete);
    }
    if (TestMEdWindowObjs[WO_MPGRP2])
    {
    // Up to now only one SelectGroup, will be more later
        /* GetAttr(MAPEDIT_Map, (Object *)TestMEdWindowObjs[WO_MPGRP2], &pt);
        SelectGroup = (struct MCMap *)pt;
        if (SelectGroup)
        {
            do
            {
                struct MCMap *tmap=NULL;
                if (SelectGroup->mm_Columns)
                {
                    int i;
                    for (i=0; i< SELYSIZE;i++) // All columns have to be freed
                    {
                        if (SelectGroup->mm_Columns[i])
                            FreeVec((APTR)SelectGroup->mm_Columns[i]);
                        SelectGroup->mm_Columns[i]=0;
                    }
                    FreeVec(SelectGroup->mm_Columns);
                    SelectGroup->mm_Columns=NULL;
                }
                tmap = SelectGroup->mm_NextLayer; // Get pointer to next layer
                SelectGroup->mm_NextLayer = NULL; // Do not reference it again!
                FreeVec(SelectGroup);        // Free this layer
                SelectGroup=tmap;     // Set new current layer
            } while (SelectGroup);
        } */
        DoGadgetMethod((struct Gadget *)TestMEdWindowObjs[WO_MPGRP2],window,NULL,MAP_Delete);
    }
    if (TestMEdWindowObjs[WO_MPGRP3])
    {
    // Up to now only one SelectGroup, will be more later
        /* GetAttr(MAPEDIT_Map, (Object *)TestMEdWindowObjs[WO_MPGRP3], &pt);
        SelectGroup = (struct MCMap *)pt;
        if (SelectGroup)
        {
            do
            {
                struct MCMap *tmap=NULL;
                if (SelectGroup->mm_Columns)
                {
                    int i;
                    for (i=0; i< SELYSIZE;i++) // All columns have to be freed
                    {
                        if (SelectGroup->mm_Columns[i])
                            FreeVec((APTR)SelectGroup->mm_Columns[i]);
                        SelectGroup->mm_Columns[i]=0;
                    }
                    FreeVec(SelectGroup->mm_Columns);
                    SelectGroup->mm_Columns=NULL;
                }
                tmap = SelectGroup->mm_NextLayer; // Get pointer to next layer
                SelectGroup->mm_NextLayer = NULL; // Do not reference it again!
                FreeVec(SelectGroup);        // Free this layer
                SelectGroup=tmap;     // Set new current layer
            } while (SelectGroup);
        } */
        DoGadgetMethod((struct Gadget *)TestMEdWindowObjs[WO_MPGRP3],window,NULL,MAP_Delete);
    }
    if (WO_Window) DisposeObject( WO_Window );
#ifndef __SLIB
    if (MapEditClass) FreeMapEditClass( MapEditClass );
#else
    if (BGUIMapEditBase)    CloseLibrary( BGUIMapEditBase );
#endif
}

/*
**      Put up a simple requester.
**/
ULONG Req( struct Window *win, UBYTE *gadgets, UBYTE *body, ... )
{
    struct bguiRequest      req = { NULL };

    req.br_GadgetFormat     = gadgets;
    req.br_TextFormat       = body;
    req.br_Flags            = BREQF_CENTERWINDOW | BREQF_LOCKWINDOW;
    req.br_Underscore       = '_';

    return( BGUI_RequestA( win, &req, ( ULONG * )( &body + 1 )));
}

BOOL MapPicLoad(void)
/* Load the MapPieces Bitmap using datatypes */
{
    struct ColorRegister *cmap;
    struct dtFrameBox mydtFrameBox; /* Use this with DTM_FRAMEBOX method   */
    struct FrameInfo myFrameInfo;   /* For info returned from DTM_FRAMEBOX */
    struct gpLayout mygpLayout;     /* Use this with DTM_PROCLAYOUT method */

    if (!(dtobject = NewDTObject("Breakout", PDTA_Screen, scr,
                     DTA_GroupID, GID_PICTURE,
                     TAG_END) ))
    {
        printf("Couldn't create new datatype object or not a picture file\n");
        return FALSE;
    }
    mydtFrameBox.MethodID         = DTM_FRAMEBOX;
    mydtFrameBox.dtf_GInfo        = NULL;
    mydtFrameBox.dtf_ContentsInfo = NULL;
    mydtFrameBox.dtf_FrameInfo    = &myFrameInfo;
    mydtFrameBox.dtf_SizeFrameInfo= sizeof (struct FrameInfo);
    mydtFrameBox.dtf_FrameFlags   = 0L;
    DoMethodA(dtobject, (Msg)&mydtFrameBox);

    mygpLayout.MethodID   = DTM_PROCLAYOUT;
    mygpLayout.gpl_GInfo  = NULL;
    mygpLayout.gpl_Initial= 1L;

    if(!(DoMethodA(dtobject, (Msg)&mygpLayout) )) {
        printf("Couldn't perform PROC_LAYOUT\n");
        return FALSE;
    }
    GetDTAttrs(dtobject, PDTA_DestBitMap, &mMapPieces,
                                PDTA_ColorRegisters, &cmap,
                                TAG_END);
    return TRUE;
}

void InitSelectGroup (struct MCMap *SelectGroup)
{
    ULONG i,j;
    char z[256];
    sprintf(z,"InitSelectGroup start: SelectGroup=%x\n", SelectGroup);
    KPrintF(z);

    SelectGroup->mm_MapSize.x = SELXSIZE;
    SelectGroup->mm_MapSize.y = SELYSIZE;
    SelectGroup->mm_MapSize.l = 2;
    SelectGroup->mm_Copy = FALSE;
    SelectGroup->mm_Size = sizeof(struct MCMap);
    SelectGroup->mm_NextLayer->mm_MapSize.x = SELXSIZE;
    SelectGroup->mm_NextLayer->mm_MapSize.y = SELYSIZE;
    SelectGroup->mm_NextLayer->mm_MapSize.l = 1; // Important!
    SelectGroup->mm_NextLayer->mm_Copy = FALSE;
    SelectGroup->mm_NextLayer->mm_Size = sizeof(struct MCMap);
    SelectGroup->mm_Columns = AllocVec(sizeof(ULONG)*SELYSIZE, MEMF_CLEAR);
    SelectGroup->mm_NextLayer->mm_Columns = AllocVec(sizeof(ULONG)*SELYSIZE, MEMF_CLEAR);
    sprintf(z,"InitSelectGroup Alloc: SelctGroup=%x, SelectGroup->mm_Columns=%x\n", SelectGroup, SelectGroup->mm_Columns);
    KPrintF(z);
    if (!SelectGroup->mm_Columns || !SelectGroup->mm_NextLayer->mm_Columns)
    {
        printf("Could not allocate memory for SelectGroup\n");
        CloseAll();
        exit(20);
    }
    for(i=0;i<SELYSIZE;i++)
    {
        SelectGroup->mm_Rows = AllocVec(sizeof(struct MapPiece)*SELXSIZE, MEMF_CLEAR);
        SelectGroup->mm_NextLayer->mm_Rows = AllocVec(sizeof(struct MapPiece)*SELXSIZE, MEMF_CLEAR);
        sprintf(z,"InitSelectGroup for: SelctGroup->mm_Rows=%x,SelectGroup->mm_NextLayer->mm_Rows=%x\n", SelectGroup->mm_Rows,SelectGroup->mm_NextLayer->mm_Rows);
        KPrintF(z);
        if (!SelectGroup->mm_Rows && SelectGroup->mm_NextLayer->mm_Rows)
        {
            printf("Could not allocate memory for SelectGroup\n");
            CloseAll();
            exit(20);
        }
        SelectGroup->mm_Columns[i] = (ULONG)SelectGroup->mm_Rows;
        SelectGroup->mm_NextLayer->mm_Columns[i] = (ULONG)SelectGroup->mm_NextLayer->mm_Rows;
        for(j=0;j<SELXSIZE;j++)
        {
            SelectGroup->mm_Rows[j].mp_Coordinates.x = j * 36;
            SelectGroup->mm_Rows[j].mp_Coordinates.y = i * 12;
            SelectGroup->mm_Rows[j].mp_Coordinates.l = 0;
            SelectGroup->mm_NextLayer->mm_Rows[j].mp_Coordinates.x = j * 36;
            SelectGroup->mm_NextLayer->mm_Rows[j].mp_Coordinates.y = i * 12;
            SelectGroup->mm_NextLayer->mm_Rows[j].mp_Coordinates.l = 1;
            SelectGroup->mm_Rows[j].mp_BitmapNumber = i*(j+1);
            SelectGroup->mm_Rows[j].mp_Size = sizeof(struct MapPiece);
            SelectGroup->mm_NextLayer->mm_Rows[j].mp_BitmapNumber = i*(j+1)+(SELXSIZE*SELYSIZE);
            SelectGroup->mm_NextLayer->mm_Rows[j].mp_Size = sizeof(struct MapPiece);
        }
    }
}

Object *InitTestMEdWindow( void )
{
    GdkObject *win, *frobj;
    GdkObject **ar = (Object **)&TestMEdWindowObjs;
    struct MCMap *SelectGroup;
    struct MapPiece mDefault = {50, 0, { 0, 0, 0 }, { 0, 0, 0 }, 0, NULL};
    UBYTE numl = 7;
    UBYTE *InfoTxt =  ISEQ_C  "You can edit the map in the lower big Gadget\n"
                              "and select the map pieces in the upper big Gadget.\n"
                      ISEQ_B  LIBSTRING;

    SelectGroup = AllocVec(sizeof(struct MCMap), MEMF_CLEAR);
    SelectGroup->mm_NextLayer = AllocVec(sizeof(struct MCMap), MEMF_CLEAR);
    if (!SelectGroup || !SelectGroup->mm_NextLayer)
    {
        printf("Could not allocate memory for SelectGroup\n");
        CloseAll();
        exit(20);
    }
    KPrintF("MapEdit Object");
    ar[WO_EDIT] = NewObject( MapEditClass, NULL, FRM_Type, FRTYPE_BUTTON,
                                                 MAPEDIT_MapPieces, mMapPieces,
                                                 MAPEDIT_PWidth, 36,
                                                 MAPEDIT_PLength, 12,
                                                 MAPEDIT_Default, &mDefault,
                                                 MAPEDIT_MapWidth, 40,
                                                 MAPEDIT_MapLength, 80,
                             TAG_DONE );
    if (!ar[WO_EDIT])
    {
        printf("Could not create MapEdit Object\n");
        FreeVec(SelectGroup->mm_NextLayer);
        FreeVec(SelectGroup);
        CloseAll();
        exit(20);
    }
    frobj = FrameObject, ButtonFrame, EndObject;
    InitSelectGroup(SelectGroup);
    KPrintF("MapEdit Groups\n");
    ar[WO_MPGRP1] = NewObject( MapEditClass, NULL, FRM_Type, FRTYPE_BUTTON,
                                                 MAPEDIT_MapPieces, mMapPieces,
                                                 MAPEDIT_PWidth, 36,
                                                 MAPEDIT_PLength, 12,
                                                 MAPEDIT_Default, &mDefault,
                                                 MAPEDIT_MapWidth, SELXSIZE,
                                                 MAPEDIT_MapLength, SELYSIZE,
                                                 MAPEDIT_MapLayer, 2,
                                                 MAPEDIT_Map, SelectGroup,
                                                 MAPEDIT_GetPieces, TRUE,
                                                 MAPEDIT_Frame, frobj,
                                                 MAPEDIT_FrameSpace, 4,
                             TAG_DONE );
    ar[WO_MPGRP2] = NewObject( MapEditClass, NULL, FRM_Type, FRTYPE_BUTTON,
                                                 MAPEDIT_MapPieces, mMapPieces,
                                                 MAPEDIT_PWidth, 36,
                                                 MAPEDIT_PLength, 12,
                                                 MAPEDIT_Default, &mDefault,
                                                 MAPEDIT_MapWidth, SELXSIZE,
                                                 MAPEDIT_MapLength, SELYSIZE,
                                                 MAPEDIT_MapLayer, 2,
                                                 MAPEDIT_Map, SelectGroup,
                                                 MAPEDIT_GetPieces, TRUE,
                                                 MAPEDIT_Frame, frobj,
                                                 MAPEDIT_FrameSpace, 4,
                             TAG_DONE );
    ar[WO_MPGRP3] = NewObject( MapEditClass, NULL, FRM_Type, FRTYPE_BUTTON,
                                                 MAPEDIT_MapPieces, mMapPieces,
                                                 MAPEDIT_PWidth, 36,
                                                 MAPEDIT_PLength, 12,
                                                 MAPEDIT_Default, &mDefault,
                                                 MAPEDIT_MapWidth, SELXSIZE,
                                                 MAPEDIT_MapLength, SELYSIZE,
                                                 MAPEDIT_MapLayer, 2,
                                                 MAPEDIT_Map, SelectGroup,
                                                 MAPEDIT_GetPieces, TRUE,
                                                 MAPEDIT_Frame, frobj,
                                                 MAPEDIT_FrameSpace, 4,
                             TAG_DONE );

    if (!ar[WO_MPGRP1] || !ar[WO_MPGRP2] || !ar[WO_MPGRP3])
    {
        printf("Could not create MapPieces Select Object\n");
        FreeVec(SelectGroup->mm_NextLayer);
        FreeVec(SelectGroup);
        CloseAll();
        exit(20);
    }
    if(screen->Height<380) // Adaption for small screens (f.e. 640x256)
    {
       numl=1;
       strcpy(InfoTxt,ISEQ_C ISEQ_B LIBSTRING);
    }
    UnlockPubScreen(NULL, screen);

    win = WindowObject,
        WINDOW_Title, "MapEditClass Demo V1.0",
        WINDOW_ScreenTitle, "MapEditClass Demo V1.0",
        WINDOW_ScaleHeight, 30,
        WINDOW_SmartRefresh, TRUE,
        WINDOW_AutoAspect, TRUE,
        WINDOW_ShowTitle, TRUE,
        WINDOW_MasterGroup, ar[WO_MASTER] = VGroupObject,
            Spacing (4),
            GROUP_BackFill, SHINE_RASTER,
            HOffset (4),
            VOffset (4),
            EqualWidth,EqualHeight,

            StartMember, ar[WO_MEINFO] = InfoFixed( NULL, InfoTxt, NULL, numl ), FixMinHeight,
            EndMember,

            StartMember, HGroupObject,
                Spacing (4),
                HOffset (4),
                VOffset (4),
                ButtonFrame,
                FRM_Flags, FRF_RECESSED,

                StartMember, ar[WO_B] = ButtonObject,
                    ButtonFrame,
                    LAB_Label, "Map Edit Demo",
                    LAB_Underscore, '_',
                    GA_ID, WO_B,
                EndObject, FixMinHeight, EndMember,
            EndObject, Weight (1), FixHeight (1), EndMember,

            StartMember, HGroupObject,
                Spacing (4),
                HOffset (4),
                VOffset (4),

                StartMember, ar[WO_GRID] = CheckBoxObject,
                    ButtonFrame,
                    LAB_Label, "Grid",
                    LAB_Place, PLACE_LEFT,
                    LAB_Underscore, '_',
                    GA_ID, WO_GRID,
                EndObject, FixMinWidth, FixMinHeight, EndMember,
                VarSpace (10),

                StartMember, ar[WO_FRAME] = CheckBoxObject,
                    ButtonFrame,
                    LAB_Label, "Frame",
                    LAB_Place, PLACE_LEFT,
                    LAB_Underscore, '_',
                    GA_ID, WO_FRAME,
                EndObject, FixMinWidth, FixMinHeight, EndMember,
            EndObject, Weight (1), FixHeight (1), EndMember,

            StartMember, HGroupObject,

                StartMember, VGroupObject,
                    Spacing (2),
                    HOffset (4),
                    VOffset (4),

                    StartMember, ar[WO_REGX] = PropObject,
                        PGA_Total, 25,
                        PGA_Visible, 5,
                        PGA_Top, 0,
                        PGA_Total, 25,
                        PGA_Freedom, FREEHORIZ,
                        PGA_Arrows, TRUE,
                        GA_ID, WO_REGX,
                    EndObject, FixMinHeight, EndMember,

                    StartMember, HGroupObject,
                        Spacing (2),

                        StartMember, ar[WO_EDIT] /* = Button( "Unused", WO_EDIT ) */,
                        EndMember,

                        StartMember, ar[WO_REGY] = PropObject,
                            PGA_Total, 25,
                            PGA_ArrowSize, 10,
                            PGA_Visible, 5,
                            PGA_Top, 0,
                            PGA_Total, 25,
                            PGA_Arrows, TRUE,
                            GA_ID, WO_REGY,
                        EndObject, FixMinWidth, EndMember,
                    EndObject, Weight (100), EndMember,
                EndObject, Weight (100), EndMember,

                StartMember, VGroupObject,
                    Spacing (2),
                    HOffset (4),
                    VOffset (4),
                    NeXTFrame,
                    FRM_Flags, FRF_RECESSED,

                    StartMember, ar[WO_MXGAD1] = MxObject,
                        MX_TabsObject, TRUE,
                        MX_Labels, WO_MXGAD1Labels,
                        MX_LabelPlace, PLACE_LEFT,
                        GA_ID, WO_MXGAD1,
                    EndObject, Weight (1), FixHeight (1), EndMember,

                    StartMember, PageObject,

                        PageMember, VGroupObject,
                            VarSpace (10),

                            StartMember, ar[WO_MPGRP1],
                            Weight (100), EndMember,
                        EndObject,

                        PageMember, VGroupObject,
                            VarSpace (10),

                            StartMember, ar[WO_MPGRP2], 
                            Weight (100), EndMember,
                        EndObject,

                        PageMember, VGroupObject,
                            VarSpace (10),

                            StartMember, ar[WO_MPGRP3],
                            Weight (100), EndMember,
                        EndObject,
                    EndObject, EndMember,
                EndObject, Weight (1), EndMember,
            EndObject, EndMember,

            StartMember, HGroupObject,
                Spacing (4),
                HOffset (4),
                VOffset (4),

                StartMember, ar[WO_LOAD] = ButtonObject,
                    ButtonFrame,
                    LAB_Label, "_Load",
                    LAB_Underscore, '_',
                    GA_ID, WO_LOAD,
                EndObject, FixMinHeight, EndMember,

                StartMember, ar[WO_SAVE] = ButtonObject,
                    ButtonFrame,
                    LAB_Label, "_Save",
                    LAB_Underscore, '_',
                    GA_ID, WO_SAVE,
                EndObject, FixMinHeight, EndMember,

                StartMember, ar[WO_QUIT] = ButtonObject,
                    ButtonFrame,
                    LAB_Label, "_Quit",
                    LAB_Underscore, '_',
                    GA_ID, WO_QUIT,
                EndObject, FixMinHeight, EndMember,
            EndObject, FixHeight (1), EndMember,
        EndObject,
    EndObject;

    return( win );
}

int main( int argc, char **argv )
{
    ULONG            signal, rc, tmp = 0;
    BOOL             running = TRUE;
        /* struct Screen *screen = LockPubScreen(NULL); */

#ifndef __SLIB
    if (! (MapEditClass = InitMapEditClass()) ) {
#else
    if ( ! (BGUIMapEditBase = OpenLibrary( "gadgets/mapedit_bgui.gadget", 39 )))
    {
#endif
        printf("Could not init Map Edit Class.\n");
        CloseAll();
        exit(20);
    }
#ifdef __SLIB
    MapEditClass = MAPEDIT_GetClassPtr();
#endif
    /* if (!screen)
    {
        printf("Could not lock Public Screen.\n");
        CloseAll();
        exit(20);
    }

    KPrintF("Trying to allocate BitMap..");
    mMapPieces = BGUI_AllocBitMap(680,480, 5, BMF_CLEAR | GetBitMapAttr(screen->RastPort.BitMap,BMA_FLAGS), NULL);
    if (!mMapPieces)
    {
        printf("Could not allocate MapPieces Bitmap.\n");
        CloseAll();
        exit(20);
    }
    KPrintF(".Done\n");
    UnlockPubScreen(NULL, screen); */

    KPrintF("Trying to load MapPieces Bitmap..");
    if (!MapPicLoad())
    {
        printf("Could not load MapPieces Bitmap.\n");
        CloseAll();
        exit(20);
    }
    KPrintF(".Done\nTrying to initialize Window..");

    if (!(WO_Window=InitTestMEdWindow()))
    {
        printf("Could not initialize Window.\n");
        CloseAll();
        exit(20);
    }
    KPrintF(".Done\n");

    tmp += AddMap( (Object *)TestMEdWindowObjs[WO_EDIT], TestMEdWindowObjs[WO_REGX], e2rx);
    tmp += AddMap( (Object *)TestMEdWindowObjs[WO_EDIT], TestMEdWindowObjs[WO_REGY], e2ry);
    if (tmp < 2) {
        printf("Could not assign Gadget map.\n");
        CloseAll();
        exit(20);
    }
    KPrintF("Trying to open window..");

    if (!(window=WindowOpen(WO_Window))) {
        printf("Could not open window\n");
        CloseAll();
        exit(20);
    }
    KPrintF(".Done\n");
    /* Main Loop */

    KPrintF("Main Loop\n");
    GetAttr(WINDOW_SigMask, WO_Window, &signal);
    do
    {
         Wait(signal);
         while ((rc=HandleEvent(WO_Window)) != WMHI_NOMORE) {
             switch(rc) {
                 case WMHI_CLOSEWINDOW:
                 case WO_QUIT:
                     running=FALSE;
                     break;
             }
         }
    } while(running);

    KPrintF("Exiting..");
    CloseAll();
    KPrintF(".Done\n");
    exit(0);
}

