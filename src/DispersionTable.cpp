#include "DispersionTable.h"

const DispersionTableClass dispersion_table_class{DISP_SAMPLE_TABLE, "Tabular", "table"};

unique_ptr<Dispersion> DispersionTableClass::read(Lexer& lexer) const {
    return nullptr;
}

complex DispersionTable::n_value(double wavelength) const {
    return m_interp_table.interpolate(wavelength);
}

complex DispersionTable::n_value_deriv(double wavelength, complex der[]) const {
    return 0.0;
}

int DispersionTable::fp_number() const { return 0; }

int DispersionTable::write(Writer& w) const {
    w << "table" << Writer::quoted_string{m_name} << m_interp_table.length();
    w.newline_enter();
    w << m_interp_table.table();
    w.newline_exit();
    return 0;
}

struct FileHandler {
    FileHandler(): f(nullptr) {}

    ~FileHandler() {
        if (f != nullptr) {
            fclose(f);
        }
    }

    void open(const char *filename, const char *mode) {
        f = fopen(filename, mode);
        if(f == nullptr) throw std::invalid_argument("file not found");
    }

    FILE *f;
};

unique_ptr<DispersionTable> DispersionTable::load_mat_file(const char * filename) {
    using matrix_type = MatrixArray<double, layout_col_major>;

    enum {
        WL_UNIT_DEFAULT = 0, /* Nanometers. */
        WL_UNIT_CONVERT_EV = 1,
        WL_UNIT_CONVERT_ANGSTROMS = 1 << 1,
    };

    FileHandler file_handler;
    file_handler.open(filename, "r");
    FILE *f = file_handler.f;

    str name = str::getline(f);
    str row = str::getline(f);

    int dtype, wl_unit_conv = 0;
    if (str::case_compare(row, "CAUCHY") == 0) {
        dtype = DISP_CAUCHY;
    } else if(str::case_compare(row, "EV") == 0) {
        wl_unit_conv |= WL_UNIT_CONVERT_EV;
    } else if(str::case_compare(row, "ANGSTROMS") == 0) {
        wl_unit_conv |= WL_UNIT_CONVERT_ANGSTROMS;
    } else if(str::case_compare(row, "NM") != 0) {
        throw std::invalid_argument("unknown MAT file");
    }

    int provide_diel_k = 0;
    row = str::getline(f);
    if(str::case_compare(row, "NK") == 0) {
        dtype = DISP_SAMPLE_TABLE;
    } else if(str::case_compare(row, "E1E2") == 0) {
        dtype = DISP_SAMPLE_TABLE;
        provide_diel_k = 1;
    } else {
        throw std::invalid_argument("unknown MAT file");
    }

    switch(dtype) {
    case DISP_SAMPLE_TABLE: {
        long start_pos = ftell(f);
        int lines;

        start_pos = ftell(f);

        for(lines = 0; ;) {
            float xd[3];
            int read_status = fscanf(f, "%f %f %f\n", xd, xd+1, xd+2);
            if(read_status == 3) {
                lines ++;
            }
            if(read_status == EOF) {
                break;
            }
        }

        if(lines < 2) throw std::invalid_argument("unknown or invalid MAT file");

        fseek(f, start_pos, SEEK_SET);

        shared_ptr<matrix_type> table(new matrix_type(lines, 3));

        double *wptr = table->data(0, 0);
        double *nptr = table->data(0, 1);
        double *kptr = table->data(0, 2);
        for(int j = 0; j < lines; j++, wptr++, nptr++, kptr++) {
            int read_status;
            do {
                read_status = fscanf(f, "%lf %lf %lf\n", wptr, nptr, kptr);
            } while(read_status < 3 && read_status != EOF);

            if(wl_unit_conv & WL_UNIT_CONVERT_EV) {
                *wptr = 1239.8 / *wptr;
            } else if(wl_unit_conv & WL_UNIT_CONVERT_ANGSTROMS) {
                *wptr /= 10.0;
            }

            if(provide_diel_k) {
                double e1 = *nptr, e2 = *kptr;
                double ne = sqrt(e1*e1 + e2*e2);
                *nptr = sqrt((ne + e1) / 2.0);
                *kptr = sqrt((ne - e1) / 2.0);
            }

            if(read_status == EOF) {
                break;
            }
        }

        return unique_ptr<DispersionTable>(new DispersionTable(name, table));
    }
    case DISP_CAUCHY:
        throw std::invalid_argument("cauchy MAT format is unsupported");
    default:
        throw std::invalid_argument("unknown MAT file");
    }
    return nullptr;
}
