#ifndef REGPRO_GENERIC_POD_VECTOR_H
#define REGPRO_GENERIC_POD_VECTOR_H

#include <memory>

template <typename T>
class pod_vector_base {
public:
    pod_vector_base(): number(0), m_capacity(0), m_array(nullptr) { }
    pod_vector_base(int capacity): number(0), m_capacity(capacity), m_array(std::make_unique<T[]>(capacity)) { }

    pod_vector_base(const pod_vector_base& src) {
        number = src.number;
        m_capacity = number;
        m_array = std::make_unique<T[]>(m_capacity);
        std::copy(&src.m_array[0], &src.m_array[number], &m_array[0]);
    }

    void add(const T& elem) {
        capacity(number + 1);
        m_array[number] = elem;
        number++;
    }

    void erase(const int pos) {
        std::copy(&m_array[pos + 1], &m_array[number], &m_array[pos]);
        number--;
    }

    void insert(const T& elem, const int pos) {
        capacity(number + 1);
        std::copy_backward(&m_array[pos], &m_array[number], &m_array[number + 1]);
        m_array[pos] = elem;
        number++;
    }

    void clear() { number = 0; }

    void capacity(int new_capacity) {
        if (new_capacity > m_capacity) {
            resize(new_capacity);
        }
    }

    const T& operator [] (int i) const { return m_array[i]; }
          T& operator [] (int i)       { return m_array[i]; }
    const T& at(int i) const           { return m_array[i]; }
          T& at(int i)                 { return m_array[i]; }
    T  value_at(int i) const           { return m_array[i]; }

    int number;

private:
    void resize(int new_capacity) {
        auto new_array = std::make_unique<T[]>(new_capacity);
        std::copy(&m_array[0], &m_array[number], &new_array[0]);
        m_array.swap(new_array);
    }

    int m_capacity;
    std::unique_ptr<T[]> m_array;
};

#endif
