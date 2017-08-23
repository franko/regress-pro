#ifndef REGPRO_STACK_ARRAY_H
#define REGPRO_STACK_ARRAY_H

#include <cstddef>

template <typename T, unsigned stack_capacity>
class StackArray {
public:
    StackArray(unsigned size) {
        if (size > stack_capacity) {
            m_data = new T[size];
        } else {
            m_data = m_stack;
        }
    }

    ~StackArray() {
        if (m_data != m_stack) {
            delete [] m_data;
        }
    }

    const T* data() const { return m_data; }
          T* data()       { return m_data; }

    const T& operator[](unsigned index) const { return m_data[index]; }
          T& operator[](unsigned index)       { return m_data[index]; }

    const T& at(unsigned index) const { return m_data[index]; }
          T& at(unsigned index)       { return m_data[index]; }
private:
    T m_stack[stack_capacity];
    T *m_data;
};

template <typename T, unsigned stack_capacity>
class StackArraySized : public StackArray<T, stack_capacity> {
public:
    StackArraySized(unsigned size): StackArray<T, stack_capacity>(size), m_size(size) { }
    unsigned size() const { return m_size; }
private:
    unsigned m_size;
};

#endif
