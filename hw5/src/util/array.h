/****************************************************************************
  FileName     [ array.h ]
  PackageName  [ util ]
  Synopsis     [ Define dynamic array package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef ARRAY_H
#define ARRAY_H

#include <cassert>

// NO need to implement class ArrayNode
//
template <class T>
class Array
{
public:
   Array() : _data(0), _size(0), _capacity(0) {}
   ~Array() { delete []_data; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class Array;

   public:
      iterator(T* n= 0): _node(n) {}
      iterator(const iterator& i): _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const 
      {
         return *_node;
      }
      T& operator * () { return (*_node); }
      iterator& operator ++ () // prefix
      {
         this->_node++;
         return *this;
      }
      iterator operator ++ (int) // suffix
      {
         iterator temp = *this;
         this->_node++;
         return temp;
      }
      iterator& operator -- ()
      {
         this->_node--;
         return (*this);
      }
      iterator operator -- (int)
      {
         iterator temp = *this;
         this->_node--;
         return temp;
      }

      iterator operator + (int i) const 
      {
         return iterator(this->_node + i); 
      }
      iterator& operator += (int i) 
      {
         this->_node += i;
         return (*this);
      }

      iterator& operator = (const iterator& i) 
      {
         this->_node = i._node;
         return (*this);
      }

      bool operator != (const iterator& i) const 
      {
         return (this->_node != i._node);
      }
      bool operator == (const iterator& i) const 
      {
         return (this->_node == i._node); 
      }

   private:
      T*    _node;
   };

   // TODO: implement these functions
   iterator begin() const 
   {
      return iterator(_data);
   }
   iterator end() const 
   {
      return iterator(_data + _size); 
   }
   bool empty() const 
   {
      return (_size == 0);
   }
   size_t size() const
   {
      return _size;
   }

   T& operator [] (size_t i) 
   {
      return _data[i];
   }
   const T& operator [] (size_t i) const 
   {
      return _data[i];
   }

   void pop_front()
   {
      if(_size != 0)
      {
         for(size_t i = 0 ; i<_size-1 ; i++)
         {
            _data[i] = _data[i+1];
         }
         _size--;
      }
   }
   void pop_back()
   {
      if(_size != 0)
      {
         _size--;
      }
   }

   bool erase(iterator pos)
   {
      if(_size == 0)
      {
         return false;
      }
      for(T* cur = pos._node ; cur < _data + _size - 1 ; cur++)
      {
         *cur = *(cur + 1);
      }
      _size--;
      return true;
   }
   bool erase(const T&x)
   {
      if(_size == 0)
      {
         return false;
      }
      iterator it = begin();
      for( ; it != end() ; it ++)
      {
         if(*it == x)
         {
            erase(it);
            return true;
         }
      }
      return false;
   }
   bool insert(const T& x)
   {
      // Use iterative binary search
      // http://en.wikipedia.org/wiki/Binary_search_algorithm
      // I use int instead of size_t because negative numbers require considerations, too
      int upperBound = _size - 1, lowerBound = 0;
      if(_size > 0)
      {
         while(upperBound >= lowerBound)
         {
            int midPoint = (upperBound + lowerBound)/2;
            if(_data[midPoint] < x)
            {
               lowerBound = midPoint + 1;
            }
            else if(x < _data[midPoint]) // only operator< is defined in AdtTestObj
            {
               upperBound = midPoint - 1;
            }
            else // found!!!
            {
               return false;
            }
         };
      }
      assert(upperBound + 1 == lowerBound);
      // Position: upperBound (x to be inserted) lowerBound
      if(_size+1 >= _capacity)
      {
         _capacity = (_capacity<8)?16:2*_capacity;
         // cout << "_capacity = " << _capacity << endl;
         _size++;
         T* oldData = _data;
         _data = new T[_capacity];
         if(oldData != NULL)
         {
            for(int i=0;i<=upperBound;i++)
            {
               _data[i] = oldData[i];
            }
            for(int i=lowerBound;i<=static_cast<int>(_size)-1;i++) // cast to avoid warning, assume TAs won't make elements more than 2147483647
            {
               _data[i+1] = oldData[i];
            }
            delete [] oldData;
         }
         _data[lowerBound] = x;
      }
      else
      {
         for(int i=_size-1;i>=lowerBound;i--)
         {
            _data[i+1] = _data[i];
         }
         _data[lowerBound] = x;
         _size++;
      }
      return true;
   }

   void clear()
   {
      _size = 0;
   }

   // Nice to have, but not required in this homework...
   // void reserve(size_t n) { ... }
   // void resize(size_t n) { ... }

private:
   T*           _data;
   size_t       _size;       // number of valid elements
   size_t       _capacity;   // max number of elements

   // [OPTIONAL TODO] Helper functions;
};

#endif // ARRAY_H
