#ifndef REGPRO_STACK_ARRAY_H
#define REGPRO_STACK_ARRAY_H

#include <cstddef>

template <typename T>
class StackArrayBase {
protected:
    StackArrayBase(T* data): m_data(data) { }
    StackArrayBase() = delete;
public:
    StackArrayBase(StackArrayBase& original): m_data(original.m_data) { }

    void set_elements(const T value, unsigned n) {
        for (unsigned j = 0; j < n; j++) {
            m_data[j] = value;
        }
    }

    const T* data() const { return m_data; }
          T* data()       { return m_data; }

    const T& operator[](unsigned index) const { return m_data[index]; }
          T& operator[](unsigned index)       { return m_data[index]; }

    const T& at(unsigned index) const { return m_data[index]; }
          T& at(unsigned index)       { return m_data[index]; }
protected:
    T *m_data;
};

template <typename T, unsigned Capacity>
class StackArray : public StackArrayBase<T> {
public:
    StackArray(unsigned size): StackArrayBase<T>(size > Capacity ? new T[size] : m_stack) { }

    ~StackArray() {
        T *base_data = this->m_data;
        if (base_data != m_stack) {
            delete [] base_data;
        }
    }
private:
    T m_stack[Capacity];
};

template <typename T, unsigned Capacity>
class StackArraySized : public StackArray<T, Capacity> {
public:
    StackArraySized(unsigned size): StackArray<T, Capacity>(size), m_size(size) { }
    unsigned size() const { return m_size; }
private:
    unsigned m_size;
};

#endif
