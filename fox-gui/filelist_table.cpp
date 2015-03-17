#include "filelist_table.h"

filelist_table::filelist_table(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb)
    : FXTable(p, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb), entries_no(0)
{
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
