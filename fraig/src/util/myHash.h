/****************************************************************************
  FileName     [ myHash.h ]
  PackageName  [ util ]
  Synopsis     [ Define Hash and Cache ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2009-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_H
#define MY_HASH_H

#include <vector>

using namespace std;

//--------------------
// Define Hash classes
//--------------------
// To use Hash ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class HashKey
// {
// public:
//    HashKey() {}
// 
//    size_t operator() () const { return 0; }
// 
//    bool operator == (const HashKey& k) { return true; }
// 
// private:
// };
//
template <class HashKey, class HashData>
class Hash
{
typedef pair<HashKey, HashData> HashNode;

public:
   Hash() : _numBuckets(9973), _buckets(0) { init(9973); }
   Hash(size_t b) : _numBuckets(b), _buckets(0) { init(b); }
   ~Hash() { reset(); }

   // TODO: implement the Hash<HashKey, HashData>::iterator
   // o An iterator should be able to go through all the valid HashNodes
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   // (_bId, _bnId) range from (0, 0) to (_numBuckets, 0)
   //
   class iterator
   {
      friend class Hash<HashKey, HashData>;

   public:
      iterator() : container(NULL), curBucket(NULL)
      {
      }
      iterator(const Hash* c, vector<HashNode>* b, typename vector<HashNode>::iterator it)
         : container(c), curBucket(b), itInternal(it)
      {
      }
      ~iterator()
      {
      }
      const HashData& operator*() const
      {
         return itInternal->second;
      }
      HashData& operator*()
      {
         return itInternal->second;
      }
      iterator& operator++() // prefix
      {
         while(1)
         {
            if(itInternal+1 == curBucket->end())
            {
               if((curBucket = container->getNextAvailBucket(curBucket)) != NULL)
               {
                  itInternal = curBucket->begin();
                  break;
               }
               else
               {
                  iterator tmp = container->end();
                  this->curBucket = tmp.curBucket;
                  this->itInternal = tmp.itInternal;
                  break;
               }
            }
            else
            {
               itInternal++;
               break;
            }
         }
         return *this;
      }
      iterator operator++(int) // suffix
      {
         iterator temp = *this;
         this->operator++(); // call prefix version
         return temp;
      }
      iterator& operator--()
      {
         while(1)
         {
            if(itInternal == curBucket->begin())
            {
               if((curBucket = container->getPrevAvailBucket(curBucket)) != NULL)
               {
                  itInternal = curBucket->end()-1;
                  break;
               }
               else
               {
                  iterator tmp = container->begin();
                  this->curBucket = tmp.curBucket;
                  this->itInternal = tmp.itInternal;
                  break;
               }
            }
            else
            {
               itInternal--;
               break;
            }
         }
         return *this;
      }
      iterator operator--(int)
      {
         iterator temp = *this;
         this->operator--();
         return temp;
      }
      iterator& operator=(const iterator& i)
      {
         this->container = i->container;
         this->curBucket = i->curBucket;
         this->itInternal = i->itInternal;
      }
      bool operator!=(const iterator& i) const
      {
         return this->itInternal != i.itInternal;
      }
      bool operator==(const iterator& i) const
      {
         return this->itInternal == i.itInternal;
      }
   private:
      const Hash<HashKey, HashData>* container;
      vector<HashNode>* curBucket;
      typename vector<HashNode>::iterator itInternal;
   };

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const
   {
      vector<HashNode>* availBucket = _buckets->empty()?getNextAvailBucket(_buckets):_buckets;
      if(availBucket == NULL)
      {
         vector<HashNode>* lastBucket = _buckets + _numBuckets - 1;
         return iterator(this, lastBucket, lastBucket->end());
      }
      return iterator(this, availBucket, availBucket->begin());
   }
   // Pass the end
   iterator end() const
   {
      vector<HashNode>* lastBucket = _buckets + _numBuckets - 1;
      vector<HashNode>* availBucket = lastBucket->empty()?getPrevAvailBucket(lastBucket):lastBucket;
      if(availBucket == NULL)
      {
         return iterator(this, lastBucket, lastBucket->end());
      }
      return iterator(this, availBucket, availBucket->end());
   }
   // return true if no valid data
   bool empty() const
   {
      for(unsigned int i = 0;i < _numBuckets;i++)
      {
         if(!_buckets[i].empty())
         {
            return false;
         }
      }
      return true;
   }
   // number of valid data
   size_t size() const
   {
      size_t s = 0;
      for(unsigned int i = 0;i < _numBuckets;i++)
      {
         s += _buckets[i].size();
      }
      return s;
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<HashNode>& operator [] (size_t i) { return _buckets[i]; }
   const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

   void init(size_t b)
   {
      _buckets = new vector<HashNode>[b];
   }
   void reset()
   {
      delete [] _buckets;
   }

   // check if k is in the hash...
   // if yes, update n and return true;
   // else return false;
   bool check(const HashKey& k, HashData& n)
   {
      vector<HashNode>* curBucket = _buckets + k() % _numBuckets;
      if(curBucket->empty())
      {
         return false;
      }
      for(typename vector<HashNode>::const_iterator it = curBucket->begin();it != curBucket->end();it++)
      {
         if(it->first == k)
         {
            n = it->second;
            return true;
         }
      }
      return false;
   }

   // return true if inserted successfully (i.e. k is not in the hash)
   // return false is k is already in the hash ==> will not insert
   bool insert(const HashKey& k, const HashData& d)
   {
      vector<HashNode>* curBucket = _buckets + k() % _numBuckets;
      for(typename vector<HashNode>::iterator it = curBucket->begin();it != curBucket->end();it++)
      {
         if(it->first == k)
         {
            return false;
         }
      }
      if(curBucket->size() == curBucket->capacity())
      {
         curBucket->reserve(2*curBucket->capacity());
      }
      curBucket->push_back(make_pair(k, d));
      return true;
   }

   // return true if inserted successfully (i.e. k is not in the hash)
   // return false is k is already in the hash ==> still do the insertion
   bool replaceInsert(const HashKey& k, const HashData& d)
   {
      vector<HashNode>* curBucket = _buckets + k() % _numBuckets;
      for(typename vector<HashNode>::iterator it = curBucket->begin();it != curBucket->end();it++)
      {
         if(it->first == k)
         {
            it->second = d;
            return false;
         }
      }
      curBucket->push_back(make_pair(k, d));
      return true;
   }

   // Need to be sure that k is not in the hash
   void forceInsert(const HashKey& k, const HashData& d)
   {
      _buckets[k() % _numBuckets].push_back(make_pair(k, d));
   }

   // when content changed, need to move to a new place
   /*void reCalculateHash(const HashKey& oldKey, const HashKey& newKey) 
   {
      vector<HashNode> *oldBucket = _buckets + oldKey() % _numBuckets, 
                       *newBucket = _buckets + newKey() % _numBuckets;
      HashData oldData;
      for(typename vector<HashNode>::iterator it = oldBucket->begin();it != oldBucket->end();)
      {
         if(it->first == oldKey)
         {
            oldData = it->second;
            it = oldBucket->erase(it);
            break;
         }
         else
         {
            it++;
         }
      }
      newBucket->push_back(make_pair(newKey, oldData));
   }*/

private:
   /*
    * Helper functions
    * */
   vector<HashNode>* getNextAvailBucket(vector<HashNode>* b) const // for use of iterator only
   {
      if(b == _buckets + _numBuckets - 1)
      {
         return NULL;
      }
      for(vector<HashNode>* cur = b+1;cur <= _buckets + _numBuckets - 1;cur++)
      {
         if(!cur->empty())
         {
            return cur;
         }
      }
      return NULL;
   }

   vector<HashNode>* getPrevAvailBucket(vector<HashNode>* b) const // for use of iterator only
   {
      if(b == _buckets)
      {
         return NULL;
      }
      for(vector<HashNode>* cur = b-1;cur >= _buckets;cur--)
      {
         if(!cur->empty())
         {
            return cur;
         }
      }
      return NULL;
   }

public:
   void printAll() const
   {
      for(unsigned int i = 0;i < _numBuckets;i++)
      {
         cout << "[" << i << "] ";
         if(!_buckets[i].empty())
         {
            for(typename vector<HashNode>::iterator it = _buckets[i].begin();it != _buckets[i].end();it++)
            {
               cout << it->second << "(" << it->first() << ")" << " ";
            }
         }
         cout << "\n";
      }
   }

private:
   // Do not add any extra data member
   size_t                   _numBuckets;
   vector<HashNode>*        _buckets;

   size_t bucketNum(const HashKey& k) const {
      return (k() % _numBuckets); }

};


//---------------------
// Define Cache classes
//---------------------
// To use Cache ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class CacheKey
// {
// public:
//    CacheKey() {}
//    
//    size_t operator() () const { return 0; }
//   
//    bool operator == (const CacheKey&) const { return true; }
//       
// private:
// }; 
// 
template <class CacheKey, class CacheData>
class Cache
{
typedef pair<CacheKey, CacheData> CacheNode;

public:
   Cache() : _size(9973), _cache(0) { init(9973); }
   Cache(size_t s) : _size(s), _cache(0) { init(s); }
   ~Cache() { reset(); }

   // NO NEED to implement Cache::iterator class

   // TODO: implement these functions
   //
   // Initialize _cache with size s
   void init(size_t s)
   {
      _cache = new CacheNode[s];
   }
   void reset()
   {
      delete [] _cache;
   }

   size_t size() const { return _size; }

   CacheNode& operator [] (size_t i) { return _cache[i]; }
   const CacheNode& operator [](size_t i) const { return _cache[i]; }

   // return false if cache miss
   bool read(const CacheKey& k, CacheData& d) const
   {
      size_t idx = k() % _size;
      if(!(_cache[idx].first == k)) // cache miss
      {
         return false;
      }
      else
      {
         d = _cache[idx].second;
         return true;
      }
   }
   // If k is already in the Cache, overwrite the CacheData
   void write(const CacheKey& k, const CacheData& d)
   {
      _cache[k() % _size] = make_pair(k, d);
   }

private:
   // Do not add any extra data member
   size_t         _size;
   CacheNode*     _cache;
};


#endif // MY_HASH_H
