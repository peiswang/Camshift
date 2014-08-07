#ifndef _CIRCULARQUEUE_H_
#define _CIRCULARQUEUE_H_

template <typename T> 
class CircularQueue 
{ 
private: 
    T *_pQueue; 
    int _rear, _size, _count; 
    static const int _MAX_QUEUE_SIZE = 5;
  
public: 
    CircularQueue(const int size = _MAX_QUEUE_SIZE) 
        : _rear(0) , _size(size) , _count(0)
    { _pQueue = new T[_size]; } 
      
      
    ~CircularQueue(void) 
    { delete[] _pQueue; } 
      
      
    int getSize(void) const
    { return _size; } 

	int getCount(void) const
	{ return _count; }
      
    T enqueue(const T &val) 
    { 
        T tmp;
        if(_count==_size)
             tmp = _pQueue[_rear];
        _pQueue[_rear] = val; 
        _rear = (_rear + 1) % _size; 
		if(++_count>=_size)
			_count = _size;
        return tmp;
    } 

	void clear()
	{
		_count = 0;
		_rear = 0;
	}
    
}; 

#endif
