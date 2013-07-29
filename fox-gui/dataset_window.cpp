#include "dataset_window.h"

// Map
FXDEFMAP(dataset_window) dataset_window_map[]= {
    // FXMAPFUNC(SEL_CLOSE,FXMainWindow::ID_CLOSE,dataset_window::on_cmd_close),
};

// Object implementation
FXIMPLEMENT(dataset_window,FXMainWindow,dataset_window_map,ARRAYNUMBER(dataset_window_map));

dataset_window::dataset_window(FXApp* a,const FXString& name,FXIcon *ic,FXIcon *mi,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs)
    : FXMainWindow(a, "Dispersion Fit", ic, mi, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
{
    // Menubar
    menubar = new FXMenuBar(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X);

    // fit menu
    filemenu = new FXMenuPane(this);
    new FXMenuCommand(filemenu, "Load File", NULL, this, ID_LOAD_FILE);
    new FXMenuCommand(filemenu, "Load Multiple Files", NULL, this, ID_LOAD_FILES);
    new FXMenuTitle(menubar, "File", NULL, filemenu);

    manipmenu=new FXMenuPane(this);
    new FXMenuCommand(manipmenu,"Edit Cell",NULL,table,FXTable::ID_START_INPUT);
    new FXMenuCommand(manipmenu,"Delete Column",NULL,table,FXTable::ID_DELETE_COLUMN);
    new FXMenuCommand(manipmenu,"Delete Row",NULL,table,FXTable::ID_DELETE_ROW);
    new FXMenuCommand(manipmenu,"Insert Column",NULL,table,FXTable::ID_INSERT_COLUMN);
    new FXMenuCommand(manipmenu,"Insert Row",NULL,table,FXTable::ID_INSERT_ROW);
    // new FXMenuCommand(manipmenu,"Resize table...",NULL,this,TableWindow::ID_RESIZETABLE);
    new FXMenuTitle(menubar,"&Manipulations",NULL,manipmenu);

    // Contents
    FXVerticalFrame* contents=new FXVerticalFrame(this,LAYOUT_SIDE_TOP|FRAME_NONE|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    FXVerticalFrame* frame=new FXVerticalFrame(contents,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);

    // Table
    table=new FXTable(frame,NULL,0,TABLE_COL_SIZABLE|TABLE_ROW_SIZABLE|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 2,2,2,2);
    table->setVisibleRows(20);
    table->setVisibleColumns(8);
    table->setTableSize(20, 8);

    table->setColumnText(0, "Spectra File");
}

dataset_window::~dataset_window()
{
    delete filemenu;
    delete manipmenu;
}

FXbool dataset_window::close(FXbool notify)
{
    fprintf(stderr, "ON CMD CLOSE\n");
    hide();
    return FALSE;
}
