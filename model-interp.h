#ifndef MODEL_INTERP_H
#define MODEL_INTERP_H

#include "interp.h"

extern obj_t * cauchymodel_interp(struct symtab *st, struct gen_record *r);
extern obj_t * ho_model_interp(struct symtab *st, struct gen_record *r);
extern obj_t * read_mat_file_interp(struct symtab *st, struct gen_record *r);
extern obj_t * read_nk_file_interp(struct symtab *st, struct gen_record *r);
extern obj_t * lookup_model_interp(struct symtab *st, struct gen_record *r);
extern obj_t * bruggeman_model_interp(struct symtab *st, struct gen_record *r);

#endif
