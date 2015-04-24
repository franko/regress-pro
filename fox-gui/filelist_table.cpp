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

void filelist_table::set_filename(int i, const char *filename)
{
    char rowlabel[64];
    sprintf(rowlabel, "%d", i + 1);
    setRowText(i, rowlabel);
    setItemText(i, 0, filename);
}

void filelist_table::append_rows(int n)
{
    if (entries_no + n > getNumRows()) {
        int add_no = entries_no + n - getNumRows();
        insertRows(entries_no, add_no);
    }
    entries_no += n;
}

void filelist_table::clear_samples()
{
    removeRows(0, entries_no);
    insertRows(0, FILELIST_MIN_ROWS);
    entries_no = 0;
}

long filelist_table::on_cmd_add_files(FXObject *, FXSelector, void *)
{
    FXFileDialog open(this,"Open Spectra");
    open.setSelectMode(SELECTFILE_MULTIPLE_ALL);
    open.setPatternList("Spectra File (*.dat)\nAny Files (*)");
    if (open.execute()) {
        FXString *filenames = open.getFilenames();
        int count = 0;
        for (int i = 0; filenames && filenames[i] != ""; i++) {
            count++;
        }
        int n = entries_no;
        append_rows(count);
        for (int i = 0; filenames && filenames[i] != ""; i++) {
            set_filename(n + i, filenames[i].text());
        }
        delete [] filenames;
    }
    return 1;
}

long filelist_table::on_cmd_remove_files(FXObject *, FXSelector, void *)
{
    return 0;
}
