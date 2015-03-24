#include "filelist_table.h"

// Map
FXDEFMAP(filelist_table) filelist_table_map[]= {
    FXMAPFUNC(SEL_COMMAND, filelist_table::ID_ADD_FILES, filelist_table::on_cmd_add_files),
    FXMAPFUNC(SEL_COMMAND, filelist_table::ID_REMOVE_FILES, filelist_table::on_cmd_remove_files),
};

FXIMPLEMENT(filelist_table,FXTable,filelist_table_map,ARRAYNUMBER(filelist_table_map));

filelist_table::filelist_table(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb)
    : FXTable(p, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb), entries_no(0)
{
    setTableSize(FILELIST_MIN_ROWS, FILELIST_MIN_COLUMNS);
    setRowHeaderWidth(28);
    setColumnText(0, "Filename");
}

void filelist_table::append_filenames(FXString *filenames)
{
    int count = 0;
    for(FXString *p = filenames; *p != ""; p++, count++) ;
    if (entries_no + count > getNumRows()) {
        int del = entries_no + count - getNumRows();
        insertRows(entries_no, del);
    }
    char rowlabel[64];
    for (int i = 0; filenames[i] != ""; i++) {
        int n = entries_no + i;
        sprintf(rowlabel, "%d", n + 1);
        setRowText(n, rowlabel);
        setItemText(n, 0, filenames[i]);
    }
    entries_no += count;
}

long filelist_table::on_cmd_add_files(FXObject *, FXSelector, void *)
{
    FXFileDialog open(this,"Open Spectra");
    open.setSelectMode(SELECTFILE_MULTIPLE_ALL);
    open.setPatternList("Spectra File (*.dat)\nAny Files (*)");
    if (open.execute()) {
        FXString *filenames = open.getFilenames();
        if (filenames) {
            append_filenames(filenames);
        }
    }
    return 1;
}

long filelist_table::on_cmd_remove_files(FXObject *, FXSelector, void *)
{
    return 0;
}
