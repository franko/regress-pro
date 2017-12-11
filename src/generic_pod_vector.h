#ifndef REGPRO_GENERIC_POD_VECTOR_H
#define REGPRO_GENERIC_POD_VECTOR_H

#include <memory>

template <unsigned BlockSize>
struct AllocatorPolicyBlockSize {
    static int capacity(int new_capacity) {
        return (new_capacity <= 0 ? BlockSize : BlockSize * ((new_capacity + BlockSize - 1) / BlockSize));
    }

    static bool release_on_clear() { return false; }
};

template <typename T, typename AllocatorPolicy = AllocatorPolicyBlockSize<8> >
class pod_vector_base {
public:
    pod_vector_base(int cap = 0): number(0), m_capacity(AllocatorPolicy::capacity(cap)), m_array(std::make_unique<T[]>(AllocatorPolicy::capacity(cap))) { }

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

    void add_array(const T array[], int dim) {
        capacity(number + dim);
        for (int i = 0; i < dim; i++) {
            m_array[number + i] = array[i];
        }
        number += dim;
    }

    void erase(const int pos, const int dim = 1) {
        if (pos + dim < number) {
            std::copy(&m_array[pos + dim], &m_array[number], &m_array[pos]);
        }
        number -= dim;
    }

    void insert(const T& elem, const int pos) {
        capacity(number + 1);
        std::copy_backward(&m_array[pos], &m_array[number], &m_array[number + 1]);
        m_array[pos] = elem;
        number++;
    }

    void clear() {
        number = 0;
        if (AllocatorPolicy::release_on_clear()) {
            resize(AllocatorPolicy::capacity(0));
        }
    }

    void capacity(int new_capacity) {
        if (new_capacity > m_capacity) {
            resize(AllocatorPolicy::capacity(new_capacity));
        }
    }

    const T& operator [] (int i) const { return m_array[i]; }
          T& operator [] (int i)       { return m_array[i]; }
    const T& at(int i) const           { return m_array[i]; }
          T& at(int i)                 { return m_array[i]; }
    T  value_at(int i) const           { return m_array[i]; }

    const T* data() const { return m_array.get(); }
          T* data()       { return m_array.get(); }

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
