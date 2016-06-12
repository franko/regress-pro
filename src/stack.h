#ifndef STACK_H
#define STACK_H

#include <memory>
#include <EASTL/vector.h>

#include "cmpl.h"
#include "dispers.h"
#include "fit-params.h"
#include "Writer.h"
#include "Lexer.h"

class Stack {
public:
    struct Layer {
        std::unique_ptr<Dispersion> dispersion;
        double thickness;
    };

    using size_type = eastl::vector<Layer>::size_type;

    Stack(int n): m_layers(size_type(n)) { }
#if 0
    Stack(unique_ptr<Dispersion> substrate, unique_ptr<Dispersion> environ)
    : m_substrate(std::move(substrate)), m_environ(std::move(environ)) {
    }
#endif

    void substrate(std::unique_ptr<Dispersion>&& disp) { m_substrate = std::move(disp); }
    void environ  (std::unique_ptr<Dispersion>&& disp) { m_environ   = std::move(disp); }

    int write(Writer& w);
    static std::unique_ptr<Stack> read(Lexer& lexer);

private:
    enum { MAX_LAYERS_NUMBER = (1 << 16) - 2 };

    std::unique_ptr<Dispersion> m_substrate;
    std::unique_ptr<Dispersion> m_environ;
    eastl::vector<Layer> m_layers;
};

#if 0
__BEGIN_DECLS

struct stack {
    int nb; /* number of mediums */
    struct disp_struct ** disp;
    double *thickness;
    size_t nb_alloc;
};

typedef struct stack stack_t;

extern int      stack_apply_param(stack_t *s, const fit_param_t *fp,
                                  double val);
extern void     stack_free(stack_t *d);
extern stack_t *stack_copy(const stack_t *s);
extern void     stack_init(stack_t *s);
extern void     stack_init_raw(stack_t *s, size_t nb_init);
extern void     stack_add_layer(stack_t *s, disp_t *lyr, double th);
extern void     stack_insert_layer(stack_t *s, int pos, disp_t *lyr, double th);
extern void     stack_delete_layer(stack_t *s, int pos);
extern const
double *        stack_get_ths_list(const stack_t *s);
extern void     stack_get_ns_list(stack_t *s, cmpl *ns, double lambda);
extern void     stack_get_all_parameters(stack_t *s, struct fit_parameters *fps);
extern double   stack_get_parameter_value(const stack_t *s, const fit_param_t *fp);
extern int      stack_write(writer_t *w, const stack_t *s);
extern stack_t *stack_read(lexer_t *l);

__END_DECLS
#endif

#endif
