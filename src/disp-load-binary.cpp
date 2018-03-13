#include <stdio.h>
#include <stdlib.h>

#include "disp-load-binary.h"
#include "error-messages.h"
#include "str_cpp.h"
#include "pod_vector.h"
#include "disp-ho-build.h"
#include "disp-fb.h"
#include "math-utils.h"

const char *hoTag = "\x25\x44\xab\x7a\xfc\x02";
const char *taucLorentzTag = "\x25\x44\x47\xba\xce\x04";

struct BinaryString {
    BinaryString(const char *cont, int len): content(cont), length(len) { }
    BinaryString(const char *cont): content(cont), length(strlen(cont)) { }

    const char *content;
    int length;
};

class BinaryDispData {
public:
    BinaryDispData(const char *content, int len): m_content(content), m_pos(0), m_length(len) { }

    bool expectString(const BinaryString& s);
    bool findString(const BinaryString& s);

    disp_t *parse(str_ptr *error_msg);
private:
    const char *current() const { return m_content + m_pos; }
    void skip(int n) { m_pos += n; }
    str readLenPrefixString();
    int readIntByte();
    float readFloat32();
    bool readModelParameters(pod::array<float>& parameters, str& dispname, const int ndiscard = 0);

    const char *m_content;
    int m_pos;
    int m_length;
};

bool BinaryDispData::expectString(const BinaryString& s) {
    if (strncmp(s.content, current(), s.length) == 0) {
        skip(s.length);
        return true;
    }
    return false;
}

bool BinaryDispData::findString(const BinaryString& s) {
    for (int i = m_pos; i < m_length - s.length; i++) {
        if (strncmp(m_content + i, s.content, s.length) == 0) {
            m_pos = i + s.length;
            return true;
        }
    }
    return false;
}

int BinaryDispData::readIntByte() {
    int value = int(*current());
    m_pos += 1;
    return value;
}

float BinaryDispData::readFloat32() {
    float value = *((float *) current());
    m_pos += sizeof(float);
    return value;
}

str BinaryDispData::readLenPrefixString() {
    int slen = readIntByte();
    str value(slen);
    memcpy(value.heap, current(), slen);
    value.heap[slen] = 0;
    value.length = slen;
    skip(slen);
    return value;
}

bool BinaryDispData::readModelParameters(pod::array<float>& parameters, str& dispname, const int ndiscard) {
  skip(5);
  int nparameters = readIntByte();
  skip(4);
  dispname = readLenPrefixString();
  skip(3);
  for (int i = 0; i < ndiscard; i++) {
    readFloat32();
  }
  parameters.resize(nparameters - ndiscard);
  for (int i = 0; i < nparameters - ndiscard; i++) {
    parameters[i] = readFloat32();
  }
  return true;
}

disp_t *BinaryDispData::parse(str_ptr *error_msg) {
    if (!expectString(BinaryString("datafile\0", 9))) {
        *error_msg = new_error_message(LOADING_FILE_ERROR, "Invalid or unknown binary format");
        return nullptr;
    }
    if (findString(hoTag)) {
        pod::array<float> parameters;
        str dispname;
        readModelParameters(parameters, dispname, 3);
        const int osc_num = parameters.size() / 5;
        disp_t *new_disp = new_ho(dispname.text(), osc_num, [&parameters](int i) { return double(parameters[i]); });
        if (!new_disp) {
            *error_msg = new_error_message(LOADING_FILE_ERROR, "Error creating the HO model");
            return nullptr;
        }
        disp_set_info_wavelength(new_disp, 240.0, 780.0);
        return new_disp;
    } else if (findString(taucLorentzTag)) {
        pod::array<float> parameters;
        str dispname;
        readModelParameters(parameters, dispname, 1);
        fb_osc osc[3] = {parameters[1], parameters[3], parameters[4]};
        disp_t *new_disp = disp_new_tauc_lorentz(dispname.text(), TAUC_LORENTZ_STANDARD, 1, pow2(parameters[0]), parameters[2], osc);
        disp_set_info_wavelength(new_disp, 240.0, 780.0);
        return new_disp;
    }
    *error_msg = new_error_message(LOADING_FILE_ERROR, "Invalid or unknown binary format");
    return nullptr;
}

disp_t *disp_load_binary(const char *filename, str_ptr *error_msg) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        *error_msg = new_error_message(LOADING_FILE_ERROR, "File \"%s\" does not exists or cannot be opened", filename);
        return nullptr;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *file_content = (char *) malloc(fsize + 1);
    if (!file_content) {
        *error_msg = new_error_message(LOADING_FILE_ERROR, "Cannot allocate memory to load file.");
        fclose(f);
        return nullptr;
    }
    fread(file_content, fsize, 1, f);
    fclose(f);

    file_content[fsize] = 0;

    BinaryDispData data(file_content, fsize);
    disp_t *d = data.parse(error_msg);
    free(file_content);
    return d;
}
