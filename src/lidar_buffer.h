#ifndef BUFFER_H
#define BUFFER_H

template <typename T> 
class buffer { 
    private: 
        int dim;
    public: 
        T *arr; 
        int size;

        buffer(int s);

        void append(T);
        bool full();
        void clear();
        
        ~buffer(){
            delete[] arr;
        }
}; 
  
template <typename T> 
buffer<T>::buffer(int s) { 
    arr = new T[s]; 
    dim = s;
    size = -1;
} 

template <typename T> 
void buffer<T>::append(T t) { 
    if(full()){
        return;
    }
    arr[++size] = t;
}

template <typename T> 
bool buffer<T>::full() { 
    return size == dim;
}

template <typename T> 
void buffer<T>::clear() { 
    size = -1;
}

#endif

