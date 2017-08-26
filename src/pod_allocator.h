#ifndef REGPRO_POD_ALLOCATOR_H
#define REGPRO_POD_ALLOCATOR_H

namespace pod
{
    // The policy of all AGG containers and memory allocation strategy 
    // in general is that no allocated data requires explicit construction.
    // It means that the allocator can be really simple; you can even
    // replace new/delete to malloc/free. The constructors and destructors 
    // won't be called in this case, however everything will remain working. 
    // The second argument of deallocate() is the size of the allocated 
    // block. You can use this information if you wish.
    //------------------------------------------------------------pod_allocator
    template<class T> struct allocator
    {
        static T*   allocate(unsigned num)       { return new T [num]; }
        static void deallocate(T* ptr, unsigned) { delete [] ptr;      }
    };

    // Single object allocator. It's also can be replaced with your custom
    // allocator. The difference is that it can only allocate a single 
    // object and the constructor and destructor must be called. 
    // In AGG there is no need to allocate an array of objects with
    // calling their constructors (only single ones). So that, if you
    // replace these new/delete to malloc/free make sure that the in-place
    // new is called and take care of calling the destructor too.
    //------------------------------------------------------------obj_allocator
    template<class T> struct obj_allocator
    {
        static T*   allocate()         { return new T; }
        static void deallocate(T* ptr) { delete ptr;   }
    };

}

#endif
