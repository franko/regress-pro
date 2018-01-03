#ifndef REGPRO_STACK_ARRAY_H
#define REGPRO_STACK_ARRAY_H

#include <cstddef>

template <typename T>
class stack_array_base {
protected:
    stack_array_base(T* data): m_data(data) { }
    stack_array_base() = delete;
public:
    stack_array_base(stack_array_base& original): m_data(original.m_data) { }

    void set_elements(const T value, unsigned n) {
        for (unsigned j = 0; j < n; j++) {
            m_data[j] = value;
        }
    }

    operator const T*() const { return m_data; }
    operator       T*()      { return m_data; }

    const T* data() const { return m_data; }
          T* data()       { return m_data; }

    const T& at(unsigned index) const { return m_data[index]; }
          T& at(unsigned index)       { return m_data[index]; }
protected:
    T *m_data;
};

template <typename T, unsigned Capacity>
class stack_array : public stack_array_base<T> {
public:
    stack_array(unsigned size): stack_array_base<T>(size > Capacity ? new T[size] : m_stack) { }

    ~stack_array() {
        T *base_data = this->m_data;
        if (base_data != m_stack) {
            delete [] base_data;
        }
    }
private:
    T m_stack[Capacity];
};

template <typename T, unsigned Capacity>
class stack_array_sized : public stack_array<T, Capacity> {
public:
    stack_array_sized(unsigned size): stack_array<T, Capacity>(size), m_size(size) { }
    unsigned size() const { return m_size; }
private:
    unsigned m_size;
};

#endif
