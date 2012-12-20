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
   Hash() : _numBuckets(0), _buckets(0) {}
   Hash(size_t b) : _numBuckets(0), _buckets(0) { init(b); }
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

   private:
   };

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const { return iterator(); }
   // Pass the end
   iterator end() const { return iterator(); }
   // return true if no valid data
   bool empty() const { return true; }
   // number of valid data
   size_t size() const { size_t s = 0; return s; }
   size_t numBuckets() const { return _numBuckets; }

   vector<HashNode>& operator [] (size_t i) { return _buckets[i]; }
   const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

   void init(size_t b) { }
   void reset() { }

   // check if k is in the hash...
   // if yes, update n and return true;
   // else return false;
   bool check(const HashKey& k, HashData& n) { return false; }

   // return true if inserted successfully (i.e. k is not in the hash)
   // return false is k is already in the hash ==> will not insert
   bool insert(const HashKey& k, const HashData& d) { return true; }

   // return true if inserted successfully (i.e. k is not in the hash)
   // return false is k is already in the hash ==> still do the insertion
   bool replaceInsert(const HashKey& k, const HashData& d) { return true; }

   // Need to be sure that k is not in the hash
   void forceInsert(const HashKey& k, const HashData& d) { }

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
   Cache() : _size(0), _cache(0) {}
   Cache(size_t s) : _size(0), _cache(0) { init(s); }
   ~Cache() { reset(); }

   // NO NEED to implement Cache::iterator class

   // TODO: implement these functions
   //
   // Initialize _cache with size s
   void init(size_t s) { }
   void reset() { }

   size_t size() const { return _size; }

   CacheNode& operator [] (size_t i) { return _cache[i]; }
   const CacheNode& operator [](size_t i) const { return _cache[i]; }

   // return false if cache miss
   bool read(const CacheKey& k, CacheData& d) const { return false; }
   // If k is already in the Cache, overwrite the CacheData
   void write(const CacheKey& k, const CacheData& d) { }

private:
   // Do not add any extra data member
   size_t         _size;
   CacheNode*     _cache;
};


#endif // MY_HASH_H
