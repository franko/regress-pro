#ifndef FX_NUMERIC_FIELD_H
#define FX_NUMERIC_FIELD_H

#include <fx.h>

class fx_numeric_field : public FXTextField {
    FXDECLARE(fx_numeric_field)

protected:
    fx_numeric_field() {};
private:
    fx_numeric_field(const fx_numeric_field&);
    fx_numeric_field &operator=(const fx_numeric_field&);
public:
    fx_numeric_field(FXComposite* p,FXint ncols,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=TEXTFIELD_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD) :
        FXTextField(p,ncols,tgt,sel,opts,x,y,w,h,pl,pr,pt,pb)
    { }

    void setNumber(double x);
    double getNumber() const;

    long change_on_digit(int sign);

    long on_key_press(FXObject*, FXSelector, void*);
    long on_cmd_increment(FXObject*, FXSelector, void*);
    long on_cmd_decrement(FXObject*, FXSelector, void*);

    enum {
        ID_INCREMENT_DIGIT = FXTextField::ID_LAST,
        ID_DECREMENT_DIGIT,
        ID_LAST
    };
};

#endif
