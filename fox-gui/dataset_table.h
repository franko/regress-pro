#ifndef DATASET_TABLE_H
#define DATASET_TABLE_H

#include <fx.h>

class dataset_table : public FXTable {
    FXDECLARE(dataset_table)

protected:
    dataset_table() {};
private:
    dataset_table(const dataset_table&);
    dataset_table &operator=(const dataset_table&);

public:
    dataset_table(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_MARGIN,FXint pr=DEFAULT_MARGIN,FXint pt=DEFAULT_MARGIN,FXint pb=DEFAULT_MARGIN);

    void append_filenames(FXString *filenames);

    long on_left_button(FXObject *, FXSelector, void *);

private:
    int entries_no;
};

#endif
