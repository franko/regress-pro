#ifndef DISPERS_H
#define DISPERS_H

#include "cmpl.h"
// #include "fit-params.h"
#include "str_cpp.h"
#include "Writer.h"
#include "Lexer.h"

enum {
    DISP_UNSET = 0,
    DISP_TABLE,
    DISP_SAMPLE_TABLE,
    DISP_CAUCHY,
    DISP_HO,
    DISP_LOOKUP,
    DISP_BRUGGEMAN,
    DISP_FB,
    DISP_TAUC_LORENTZ,
    DISP_END_OF_TABLE, /* Not a dispersion type */
};

class Dispersion;

struct DispersionClass {
    int id;
    str full_name;
    str short_name;

    DispersionClass(int _id, const char *_full_name, const char *_short_name)
    : id(_id), full_name(_full_name), short_name(_short_name)
    { }

    virtual std::unique_ptr<Dispersion> read(Lexer& lexer) const = 0;
};

class Dispersion {
public:
    Dispersion(const char *name, const DispersionClass& klass)
    : m_class(klass), m_name(name)
    { }

    Dispersion(const str& name, const DispersionClass& klass)
    : m_class(klass), m_name(name)
    { }

    virtual ~Dispersion() { }

    virtual complex n_value(double lambda) const = 0;
    virtual int fp_number() const = 0;
    virtual complex n_value_deriv(double lam, complex der[]) const = 0;
    // virtual int apply_param(const FitParameter& fp, double val) = 0;
    // virtual double *map_param(int index) = 0;
    // virtual double get_param_value(const FitParameter& fp) const = 0;
    virtual int write(Writer& w) const = 0;

    const str& name() const { return m_name; }

    /* class methods */
    // virtual str encode_param(const FitParameter& fp) const = 0;

    static void register_class(const DispersionClass& d);
    static const DispersionClass *lookup_class(const char *short_name);

    static eastl::vector<const DispersionClass *> m_registered_classes;
    static std::unique_ptr<Dispersion> read(Lexer& l);

protected:
    const DispersionClass& m_class;
	str m_name;
};

#if 0
struct deriv_info {
    int is_valid;
    complex *val;
};

extern disp_t * load_nk_table(const char * filename, str_ptr *error_msg);
extern disp_t * load_mat_dispers(const char * filename, str_ptr *error_msg);
extern cmpl     n_value(const disp_t *mat, double lam);
extern void     n_value_cpp(const disp_t *mat, double lam,
                            double *nr, double *ni);
extern void     n_value_deriv(const disp_t *disp, cmpl_vector *der,
                              double lambda);
extern double * disp_map_param(disp_t *d, int index);
extern int      dispers_apply_param(disp_t *d, const fit_param_t *fp,
                                    double val);
extern void     get_model_param_deriv(const disp_t *d,
                                      struct deriv_info *der,
                                      const fit_param_t *fp, double lambda,
                                      double *dnr, double *dni);
extern void     disp_free(disp_t *d);
extern disp_t * disp_copy(const disp_t *d);
extern disp_t * disp_new(enum disp_type tp);
extern disp_t * disp_new_with_name(enum disp_type tp, const char *name);
extern int      disp_get_number_of_params(const disp_t *d);
extern double   disp_get_param_value(const disp_t *d, const fit_param_t *fp);
extern int      disp_integrity_check(disp_t *d);
extern int      disp_check_fit_param(disp_t *d, fit_param_t *fp);

extern disp_t * disp_base_copy(const disp_t *src);
extern void     disp_base_free(disp_t *d);
extern int      disp_base_fp_number(const disp_t *src);
extern int      disp_is_tabular(const disp_t *d);
extern int      disp_write(writer_t *w, const disp_t *_d);
extern disp_t * disp_read(lexer_t *l);
#endif

#endif
