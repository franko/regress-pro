#ifndef POD_ARRAY_H
#define POD_ARRAY_H

#include <utility>
#include <new>
#include <cstring>

template<class T> class pod_array
{
public:
    typedef T value_type;
    typedef pod_array<T> self_type;

    ~pod_array() { delete [] m_array; }
    pod_array() : m_array(nullptr), m_size(0) {}

    pod_array(int size) :
        m_array(new T[size]),
        m_size(size)
    {}

    pod_array(const self_type& v) :
        m_array(new T[v.m_size]),
        m_size(v.m_size)
    {
        memcpy(m_array, v.m_array, sizeof(T) * m_size);
    }

    pod_array(self_type&& v) {
        m_array = v.m_array;
        m_size = v.m_size;
        v.m_array = nullptr;
        v.m_size = 0;
    }

    void resize(int size)
    {
        if(size != m_size)
        {
            delete [] m_array;
            m_size = size;
            m_array = new T[m_size];
        }
    }

    void insert(const int index, const T new_element) {
        T *new_array = new T[m_size + 1];
        if (index > 0) {
            memcpy(new_array, m_array, sizeof(T) * index);
        }
        new_array[index] = new_element;
        if (m_size - index > 0) {
            memcpy(new_array + index + 1, m_array + index, sizeof(T) * (m_size - index));
        }
        std::swap(m_array, new_array);
        m_size += 1;
        delete new_array;
    }

    self_type& operator = (const self_type& v)
    {
        resize(v.size());
        memcpy(m_array, v.m_array, sizeof(T) * m_size);
        return *this;
    }

    self_type& operator = (self_type&& v)
    {
        m_array = v.m_array;
        m_size = v.m_size;
        v.m_array = nullptr;
        v.m_size = 0;
        return *this;
    }        

    int size() const { return m_size; }
    const T& operator [] (int i) const { return m_array[i]; }
          T& operator [] (int i)       { return m_array[i]; }
    const T& at(int i) const           { return m_array[i]; }
          T& at(int i)                 { return m_array[i]; }
    T  value_at(int i) const           { return m_array[i]; }

    const T* data() const { return m_array; }
          T* data()       { return m_array; }
private:
    T*       m_array;
    int m_size;
};

#endif
