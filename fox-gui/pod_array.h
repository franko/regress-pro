#ifndef POD_ARRAY_H
#define POD_ARRAY_H

#include <utility>
#include <new>
#include <cstring>
#include <initializer_list>

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

template<class T> struct pod_allocator
{
    static T*   allocate(unsigned num)       { return new T [num]; }
    static void deallocate(T* ptr, unsigned) { delete [] ptr;      }
};

//--------------------------------------------------------------pod_vector
// A simple class template to store Plain Old Data, a vector
// of a fixed size. The data is continous in memory
//------------------------------------------------------------------------
template<class T> class pod_vector
{
public:
    typedef T value_type;
    using size_type = int;

    ~pod_vector() { pod_allocator<T>::deallocate(m_array, m_capacity); }
    pod_vector() : m_size(0), m_capacity(0), m_array(nullptr) {}
    pod_vector(size_type cap, size_type extra_tail=0);

    // Copying
    pod_vector(const pod_vector<T>&);

    // Moving
    pod_vector(pod_vector<T>&&);

    pod_vector(std::initializer_list<T> list);

    pod_vector<T>& operator = (pod_vector<T>);

    // Set new capacity. All data is lost, size is set to zero.
    void capacity(size_type cap, size_type extra_tail=0);
    size_type capacity() const { return m_capacity; }

    // Allocate n elements. All data is lost, 
    // but elements can be accessed in range 0...size-1. 
    void allocate(size_type size, size_type extra_tail=0);

    // Resize keeping the content.
    void resize(size_type new_size);

    void zero()
    {
        memset(m_array, 0, sizeof(T) * m_size);
    }

    void add(const T& v)         { m_array[m_size++] = v; }
    void push_back(const T& v)   { m_array[m_size++] = v; }
    void insert_at(size_type pos, const T& val);
    void inc_size(size_type size) { m_size += size; }
    size_type size()      const   { return m_size; }
    size_type byte_size() const   { return m_size * sizeof(T); }
    // void serialize(int8u* ptr) const;
    // void deserialize(const int8u* data, size_type byte_size);
    const T& operator [] (size_type i) const { return m_array[i]; }
          T& operator [] (size_type i)       { return m_array[i]; }
    const T& at(size_type i) const           { return m_array[i]; }
          T& at(size_type i)                 { return m_array[i]; }
    T  value_at(size_type i) const           { return m_array[i]; }

    const T* data() const { return m_array; }
          T* data()       { return m_array; }

    void remove_all()         { m_size = 0; }
    void clear()              { m_size = 0; }
    void cut_at(size_type num) { if(num < m_size) m_size = num; }

    T* begin() const { return &m_array[0]; }
    T* end() const { return &m_array[m_size]; }

private:
    size_type m_size;
    size_type m_capacity;
    T*       m_array;
};

//------------------------------------------------------------------------
template<class T> 
void pod_vector<T>::capacity(size_type cap, size_type extra_tail)
{
    m_size = 0;
    if(cap > m_capacity)
    {
        pod_allocator<T>::deallocate(m_array, m_capacity);
        m_capacity = cap + extra_tail;
        m_array = m_capacity ? pod_allocator<T>::allocate(m_capacity) : 0;
    }
}

//------------------------------------------------------------------------
template<class T> 
void pod_vector<T>::allocate(size_type size, size_type extra_tail)
{
    capacity(size, extra_tail);
    m_size = size;
}


//------------------------------------------------------------------------
template<class T> 
void pod_vector<T>::resize(size_type new_size)
{
    if(new_size > m_size)
    {
        if(new_size > m_capacity)
        {
            T* data = pod_allocator<T>::allocate(new_size);
            memcpy(data, m_array, m_size * sizeof(T));
            pod_allocator<T>::deallocate(m_array, m_capacity);
            m_capacity = new_size;
            m_array = data;
        }
    }
    else
    {
        m_size = new_size;
    }
}

//------------------------------------------------------------------------
template<class T> pod_vector<T>::pod_vector(size_type cap, size_type extra_tail) :
    m_size(0), 
    m_capacity(cap + extra_tail), 
    m_array(pod_allocator<T>::allocate(m_capacity)) {}

//------------------------------------------------------------------------
template<class T> pod_vector<T>::pod_vector(const pod_vector<T>& v) :
    m_size(v.m_size),
    m_capacity(v.m_capacity),
    m_array(v.m_capacity ? pod_allocator<T>::allocate(v.m_capacity) : nullptr)
{
    memcpy(m_array, v.m_array, sizeof(T) * v.m_size);
}

//------------------------------------------------------------------------
template<class T> pod_vector<T>::pod_vector(pod_vector<T>&& that) :
    m_size(that.m_size),
    m_capacity(that.m_capacity),
    m_array(that.m_array)
{
    that.m_array = nullptr;
    that.m_capacity = 0;
    that.m_size = 0;
}

template<class T> pod_vector<T>::pod_vector(std::initializer_list<T> list) :
    m_size(list.size()),
    m_capacity(list.size()),
    m_array(m_capacity > 0 ? pod_allocator<T>::allocate(m_capacity) : nullptr)
{
    int i = 0;
    for (auto p = list.begin(); p != list.end(); p++, i++) {
        m_array[i] = *p;
    }
}

//------------------------------------------------------------------------
template<class T> pod_vector<T>& 
pod_vector<T>::operator = (pod_vector<T> that)
{
    std::swap(m_array, that.m_array);
    std::swap(m_capacity, that.m_capacity);
    std::swap(m_size, that.m_size);
    return *this;
}

#if 0
//------------------------------------------------------------------------
template<class T> void pod_vector<T>::serialize(int8u* ptr) const
{ 
    if(m_size) memcpy(ptr, m_array, m_size * sizeof(T)); 
}

//------------------------------------------------------------------------
template<class T> 
void pod_vector<T>::deserialize(const int8u* data, size_type byte_size)
{
    byte_size /= sizeof(T);
    allocate(byte_size);
    if(byte_size) memcpy(m_array, data, byte_size * sizeof(T));
}
#endif

//------------------------------------------------------------------------
template<class T> 
void pod_vector<T>::insert_at(size_type pos, const T& val)
{
    if(pos >= m_size) 
    {
        m_array[m_size] = val;
    }
    else
    {
        memmove(m_array + pos + 1, m_array + pos, (m_size - pos) * sizeof(T));
        m_array[pos] = val;
    }
    ++m_size;
}

#endif
