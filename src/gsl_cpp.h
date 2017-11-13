#ifndef REGPRO_GSL_CPP_H
#define REGPRO_GSL_CPP_H

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

namespace gsl {

class vector {
public:
	vector(int n) {
		m_vector = gsl_vector_alloc(n > 0 ? n : 0);
	}

	~vector() {
		gsl_vector_free(m_vector);
	}

	operator const gsl_vector*() const { return m_vector; }
	operator       gsl_vector*()       { return m_vector; }

    const double* data() const { return m_vector->data; }
          double* data()       { return m_vector->data; }

    double& operator[](int i)       { return *gsl_vector_ptr(m_vector, i); }
    double  operator[](int i) const { return gsl_vector_get(m_vector, i); }

    double& at(int i)       { return *gsl_vector_ptr(m_vector, i); }
    double  at(int i) const { return gsl_vector_get(m_vector, i); }

    int size() const { return m_vector->size; }

protected:
	gsl_vector *m_vector;
};

}

#endif
