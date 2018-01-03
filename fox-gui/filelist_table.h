#ifndef FILELIST_TABLE_H
#define FILELIST_TABLE_H

#include <fx.h>

class regress_pro;

class filelist_table : public FXTable {
    FXDECLARE(filelist_table)

protected:
    filelist_table() {};
private:
    filelist_table(const filelist_table&);
    filelist_table &operator=(const filelist_table&);

public:
    filelist_table(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_MARGIN,FXint pr=DEFAULT_MARGIN,FXint pt=DEFAULT_MARGIN,FXint pb=DEFAULT_MARGIN);

    regress_pro *regress_pro_app() { return (regress_pro *) getApp(); }

    void set_filename(int i, const char *filename);
    void append_rows(int n);
    void clear_samples();
    int samples_number() { return entries_no; }

    long on_cmd_add_files(FXObject *, FXSelector, void *);
    long on_cmd_remove_files(FXObject *, FXSelector, void *);

    enum {
        ID_ADD_FILES = FXTable::ID_LAST,
        ID_REMOVE_FILES,
        ID_LAST
    };

    enum {
        FILELIST_MIN_COLUMNS = 12,
        FILELIST_MIN_ROWS = 20,
    };

protected:
    int entries_no;
};

#endif
