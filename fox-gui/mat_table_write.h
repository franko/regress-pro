#ifndef MAT_TABLE_WRITE_H
#define MAT_TABLE_WRITE_H

#include <stdio.h>
#include <dispers.h>

class file_writer {
    FILE *m_file;
public:
    file_writer(): m_file(NULL) {}

    bool open(const char *filename) {
        m_file = fopen(filename, "w");
        if (m_file == NULL) return false;
        return true;
    }

    void write(const char *s) {
        fputs(s, m_file);
    }

    void write_line(const char *s) {
        fputs(s, m_file);
        fputs("\n", m_file);
    }

    void write_number(const char *fmt, double val) {
        fprintf(m_file, fmt, val);
    }

    void close() {
        fclose(m_file);
    }
};

class sampled_dispersion_source {
    const disp_t *m_disp;
public:
    typedef double float_type;

    sampled_dispersion_source(const disp_t *d): m_disp(d) {}
    int rows() const { return disp_samples_number(m_disp); }
    void get_row(int i, double *w, double *n, double *k) const {
        *w = disp_sample_wavelength(m_disp, i);
        const cmpl nk = n_value(m_disp, *w);
        *n =  creal(nk);
        *k = -cimag(nk);
    }
};

class uniform_sampler {
    double m_start, m_step;
    int m_length;
public:
    uniform_sampler(double a, double b, double d): m_start(a), m_step(d), m_length(floor((b-a)/d)+1) {}
    int length() const { return m_length; }
    double value_at(int i) const { return m_start + m_step * i; }
};

template <typename Sampler>
class disp_source {
    const Sampler *m_sampler;
    const disp_t *m_disp;
public:
    typedef double float_type;

    disp_source(const disp_t *d, const Sampler *sampler): m_sampler(sampler), m_disp(d) { }
    int rows() const { return m_sampler->length(); }
    void get_row(int i, double *w, double *n, double *k) const {
        *w = m_sampler->value_at(i);
        cmpl nz = n_value(m_disp, *w);
        *n = nz.data[0];
        *k = -nz.data[1];
    }
};

template <typename OutputStream, typename Source>
void mat_table_write(const char *name, OutputStream *dest, const Source *src)
{
    dest->write_line(name);
    dest->write_line("nm");
    dest->write_line("NK");
    int n = src->rows();
    for (int i = 0; i < n; i++) {
        typename Source::float_type w, n, k;
        src->get_row(i, &w, &n, &k);
        dest->write_number("%g", w);
        dest->write("\t");
        dest->write_number("%g", n);
        dest->write("\t");
        dest->write_number("%g", k);
        dest->write("\n");
    }
    dest->close();
}

#endif
