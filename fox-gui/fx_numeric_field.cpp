
#include <fx.h>
#include <fxkeys.h>

#include "fx_numeric_field.h"

FXDEFMAP(fx_numeric_field) fx_numeric_field_map[]={
  FXMAPFUNC(SEL_COMMAND,  fx_numeric_field::ID_INCREMENT_DIGIT, fx_numeric_field::on_cmd_increment),
  FXMAPFUNC(SEL_COMMAND,  fx_numeric_field::ID_DECREMENT_DIGIT, fx_numeric_field::on_cmd_decrement),
  FXMAPFUNC(SEL_KEYPRESS, 0,                                    fx_numeric_field::on_key_press),
};

// Object implementation
FXIMPLEMENT(fx_numeric_field,FXTextField,fx_numeric_field_map,ARRAYNUMBER(fx_numeric_field_map));

long fx_numeric_field::on_key_press(FXObject* obj, FXSelector sel, void* ptr)
{
  FXEvent* event= (FXEvent*) ptr;
  flags &= ~FLAG_TIP;
  if(isEnabled()){
    flags &=~ FLAG_UPDATE;
    switch (event->code) {
      case KEY_Up:
      case KEY_KP_Up:
        if(event->state & SHIFTMASK) 
	  {
	    handle(this, FXSEL(SEL_COMMAND, ID_INCREMENT_DIGIT), NULL);
	    return 1;
	  }
	break;
      case KEY_Down:
      case KEY_KP_Down:
        if(event->state & SHIFTMASK) 
	  {
	    handle(this, FXSEL(SEL_COMMAND, ID_DECREMENT_DIGIT), NULL);
	    return 1;
	  }
	break;
    default:
      /* */ ;
    }
  }

  return FXTextField::onKeyPress(obj, sel, ptr);
}

static int ipow10 (int e)
{
  int r = 1;
  for ( ; e > 0; e--)
    r *= 10;
  return r;
}

static int
parse_simple_int (const char* txt, char const ** tail)
{
  const int int0 = int('0'), int9 = int('9');
  int i, sum = 0;
  for (i = 0; txt[i]; i++)
    {
      int c = txt[i];
      if (c < int0 || c > int9)
	break;
      sum = 10 * sum + (c - int0);
    }
  *tail = txt + i;
  return sum;
}

static int
parse_number (const char* txt, int& int_part, int& frac_part, int& sign,
	      int& pow_exp, char const ** tail_f)
{
  const char *tail, *ptr;

  for (ptr = txt; *ptr; ptr++)
    if (*ptr != ' ')
      break;

  char cs = *ptr;
  if (cs == '-' || cs == '+')
    {
      sign = (cs == '-' ? -1 : 1);
      ptr ++;
    }
  else
    {
      sign = 1;
    }

  int_part = parse_simple_int (ptr, &tail);

  if (tail == ptr)
    return -1;
  
  int dot_pos = tail - txt;
  
  ptr = tail;
  if (*ptr == '.')
    {
      ptr ++;
      frac_part = parse_simple_int (ptr, tail_f);
      if (*tail_f == ptr)
	return -1;
      pow_exp = *tail_f - ptr;
    }
  else
    {
      frac_part = 0;
      pow_exp = 0;
    }

  return dot_pos;
}

static int
get_normalized_int (const char* txt, int& pow_exp, int& dot_pos)
{
  char const *tail;
  int int_part, frac_part, sign;
  dot_pos = parse_number (txt, int_part, frac_part, sign, pow_exp, &tail);
  int_part *= ipow10 (pow_exp);
  return sign * (int_part + frac_part);
}

static FXString
denormalize (int norm, int pow_exp, int& dot_pos)
{
  int nsign = (norm < 0 ? -1 : 1);
  int p = ipow10 (pow_exp);
  if (norm < 0)
    norm = -norm;
  int int_part = norm / p, frac_part = norm % p;
  char buf[64];
  sprintf (buf, "%s%d", nsign < 0 ? "-" : "", int_part);
  dot_pos = strlen (buf);

  if (pow_exp == 0)
    return FXString(buf);
    
  return FXStringFormat("%s.%.*d", buf, pow_exp, frac_part);
}

long fx_numeric_field::change_on_digit(int sign)
{
  FXString txt = getText();
  int pos = getCursorPos();

  int pow_exp, dot_pos;
  int norm = get_normalized_int (txt.text(), pow_exp, dot_pos);
  
  int pos_exp = dot_pos - pos;
  if (pos_exp < 0)
    pos_exp ++;
  int inc_abs = ipow10 (pos_exp + pow_exp);
  
  norm += sign * inc_abs;

  FXString new_txt = denormalize (norm, pow_exp, dot_pos);
  int new_pos = dot_pos - pos_exp;
  if (pos_exp < 0)
    new_pos ++;

  setText(new_txt);
  setCursorPos(new_pos);

  if (target) 
    target->tryHandle(this, FXSEL(SEL_CHANGED,message), (void*)new_txt.text());

  return 1;
}

long fx_numeric_field::on_cmd_increment(FXObject* sender, FXSelector, void*)
{
  return change_on_digit(1);
}

long fx_numeric_field::on_cmd_decrement(FXObject*, FXSelector, void*)
{
  return change_on_digit(-1);
}
