/****************************************************************************
  FileName     [ bst.h ]
  PackageName  [ util ]
  Synopsis     [ Define binary search tree package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BST_H
#define BST_H

#include <cassert>

#define ADTP_DEBUG 0
#define ADT_PERFORMANCE_TIME 0
#define ADT_PERFORMANCE_ERASE_CONDS 0

#if ADT_PERFORMANCE_TIME
   clock_t clocks[10] = { 0 };
#endif
#if ADT_PERFORMANCE_ERASE_CONDS
   int eraseConds[10] = { 0 }; // count how often each conditions in eraseInternal occurs
#endif

template <class T> class BSTree;

// BSTreeNode is supposed to be a private class. User don't need to see it.
// Only BSTree and BSTree::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//

template<class T> class BSTree;

template <class T>
class BSTreeNode
{
   // TODO: design your own class!!
   friend class BSTree<T>;
   friend class BSTree<T>::iterator;

   BSTreeNode(const T& _data, BSTreeNode *_parent, BSTreeNode *_left = NULL, BSTreeNode* _right = NULL)
      : data(_data), parent(_parent), left(_left), right(_right)
   {
   }
   ~BSTreeNode()
   {
   }

   // order are to fit parameters of constructors
   T data;
   BSTreeNode *parent;
   BSTreeNode *left;
   BSTreeNode *right;
};


template <class T>
class BSTree
{
public:
   // TODO: design your own class!!
   class iterator
   {
   public:
      iterator(BSTreeNode<T>* data, BSTreeNode<T>* hidden = NULL): node(data), hiddenNode(hidden)
      {
      }
      ~iterator()
      {
      }

      const T& operator*() const
      {
         return this->node->data;
      }
      T operator*()
      {
         return this->node->data;
      }
      const iterator& operator++() // prefix
      {
         this->node = BSTree<T>::successor(this->node);
         return *this;
      }
      iterator operator++(int)
      {
         iterator temp = *this;
         this->node = BSTree<T>::successor(this->node);
         return temp;
      }
      const iterator& operator--() // prefix
      {
         if(!this->node)
         {
            this->node = hiddenNode;
         }
         else
         {
            this->node = BSTree<T>::predecessor(this->node);
         }
         return *this;
      }
      iterator operator--(int)
      {
         iterator temp = *this;
         if(!this->node)
         {
            this->node = hiddenNode;
         }
         else
         {
            this->node = BSTree<T>::predecessor(this->node);
         }
         return temp;
      }
      iterator& operator=(const iterator& i)
      {
         this->node = i.node;
         return *this;
      }
      bool operator!=(const iterator& i) const
      {
         return (this->node != i.node);
      }
      bool operator==(const iterator& i) const
      {
         return (this->node == i.node);
      }
   private:
      BSTreeNode<T>* node;
      BSTreeNode<T>* hiddenNode; // only for end()--

      friend class BSTree<T>;
   };

   BSTree() : root(NULL), minNode(NULL), maxNode(NULL)
   {
   }
   ~BSTree()
   {
#if ADT_PERFORMANCE_TIME
      cout << "Clocks: \n";
      for(int i=0;i<10;i++)
      {
         cout << (1.0*clocks[i])/CLOCKS_PER_SEC << " ";
      }
      cout << endl;
#endif
#if ADT_PERFORMANCE_ERASE_CONDS
      cout << "eraseConds: \n";
      for(int i=0;i<10;i++)
      {
         cout << eraseConds[i] << " ";
      }
      cout << endl;
#endif
   }
   iterator begin() const
   {
      return iterator(this->minNode);
   }
   iterator end() const
   {
      return iterator(NULL, maxNode);
   }
   bool empty() const
   {
      return (this->root == NULL);
   }
   size_t size() const
   {
      return root?sizeInternal(this->root):0;
   }
   void pop_front()
   {
      if(root) // not empty
      {
         eraseInternal(minNode);
      }
   }
   void pop_back()
   {
      if(root)
      {
         eraseInternal(maxNode);
      }
   }
   bool erase(const T& i)
   {
      BSTreeNode<T>* cur = this->root;
      while(1)
      {
         if(cur->data == i)
         {
            eraseInternal(cur);
            return true;
         }
         else if(cur->data < i)
         {
            if(cur->right)
            {
               cur = cur->right;
            }
            else
            {
               return false;
            }
         }
         else // cur->data > i
         {
            if(cur->left)
            {
               cur = cur->left;
            }
            else
            {
               return false;
            }
         }
      }
   }
   bool erase(iterator pos)
   {
      if(root) // not empty
      {
         eraseInternal(pos.node);
         return true;
      }
      else
      {
         return false;
      }
   }
   bool insert(const T& i)
   {
      if(!root) // no elements yet
      {
         minNode = maxNode = root = new BSTreeNode<T>(i, NULL);
         return true;
      }
      BSTreeNode<T>* cur = this->root;
      while(1)
      {
         if(cur->data == i)
         {
            return false;
         }
         if(i < cur->data)
         {
            if(!cur->left)
            {
               cur->left = new BSTreeNode<T>(i, cur);
               if(i < minNode->data)
               {
                  minNode = cur->left;
               }
               return true;
            }
            // left occupied, find more deeply
            cur = cur->left;
         }
         if(cur->data < i)
         {
            if(!cur->right)
            {
               cur->right = new BSTreeNode<T>(i, cur);
               if(maxNode->data < i)
               {
                  maxNode = cur->right;
               }
               return true;
            }
            cur = cur->right;
         }
      }
   }
   void clear()
   {
      if(this->root)
      {
         clearInternal(this->root);
      }
   }
   void print() const // for verbose output
   {
      if(this->root) // print nothing if empty
      {
         printInternal(cout, this->root, 0);
      }
#if ADTP_DEBUG
      printPtr("root", root);
      printPtr("minNode", minNode);
      printPtr("maxNode", maxNode);
#endif // #if ADTP_DEBUG      
   }
private:
   // helper functions
   static BSTreeNode<T>* successor(BSTreeNode<T>* i)
   {
#if ADT_PERFORMANCE_TIME
      clock_t now = clock();
#endif
      BSTreeNode<T>* ret = NULL;
      if(!i->parent) // root
      {
         ret = (i->right)?(BSTree<T>::min(i->right)):(NULL);
      }
      else if(i->parent->left == i)
      {
         if(i->right)
         {
            BSTreeNode<T>* candidate = BSTree<T>::min(i->right);
            ret = (i->parent->data < candidate->data)?i->parent:candidate;
         }
         else
         {
            ret = i->parent;
         }
      }
      else
      {
         // parent's right child
         if(i->right)
         {
            ret = BSTree<T>::min(i->right);
         }
         else
         {
            // successor are from upper levels
            BSTreeNode<T>* cur = i;
            while(1)
            {
               if(!cur->parent) // come to root, means no successor
               {
                  ret = NULL;
                  break;
               }
               else if(cur->parent->left == cur)
               {
                  ret = cur->parent;
                  break;
               }
               cur = cur->parent;
            }
         }
      }
#if ADT_PERFORMANCE_TIME
      clocks[1] = clock() - now;
#endif
      return ret;
   }
   static BSTreeNode<T>* predecessor(BSTreeNode<T>* i)
   {
      if(!i->parent) // root
      {
         return (i->left)?(BSTree<T>::max(i->left)):(NULL);
      }
      if(i->parent->right == i)
      {
         if(i->left)
         {
            BSTreeNode<T>* candidate = BSTree<T>::max(i->left);
            return (candidate->data < i->parent->data)?i->parent:candidate;
         }
         else
         {
            return i->parent;
         }
      }
      // parent's right child
      if(i->left)
      {
         return BSTree<T>::max(i->left);
      }
      // predecessor are from upper levels
      BSTreeNode<T>* cur = i;
      while(1)
      {
         if(!cur->parent) // come to root, means no successor
         {
            return NULL;
         }
         if(cur->parent->right == cur)
         {
            return cur->parent;
         }
         cur = cur->parent;
      }
   }
   static BSTreeNode<T>* min(BSTreeNode<T>* i)
   {
#if ADT_PERFORMANCE_TIME
      clock_t now = clock();
#endif
      BSTreeNode<T>* cur = i;
      while(cur->left)
      {
         cur = cur->left;
      }
#if ADT_PERFORMANCE_TIME
      clocks[0] += clock() - now;
#endif
      return cur;
   }
   static BSTreeNode<T>* max(BSTreeNode<T>* i)
   {
      BSTreeNode<T>* cur = i;
      while(cur->right)
      {
         cur = cur->right;
      }
      return cur;
   }
   size_t sizeInternal(BSTreeNode<T>* i) const
   {
      // recursive
      return ( 1 + (i->left ?sizeInternal(i->left): 0) +
                   (i->right?sizeInternal(i->right):0) );
   }
   void eraseInternal(BSTreeNode<T>* i)
   {
      if(i == maxNode)
      {
         if(!i->parent)
         {
            if(i->left)
            {
               root = i->left;
               i->left->parent = NULL;
               maxNode = BSTree<T>::max(this->root);
#if ADT_PERFORMANCE_ERASE_CONDS
               eraseConds[0]++;
#endif
               delete i;
            }
            else
            {
               root = maxNode = minNode = NULL;
#if ADT_PERFORMANCE_ERASE_CONDS
               eraseConds[1]++;
#endif
               delete i;
            }
         }
         else
         {
            if(i->left)
            {
               i->parent->right = i->left;
               i->left->parent = i->parent;
#if ADT_PERFORMANCE_ERASE_CONDS
               eraseConds[2]++;
#endif
               delete i;
            }
            else
            {
               i->parent->right = NULL;
#if ADT_PERFORMANCE_ERASE_CONDS
               eraseConds[3]++;
#endif
               delete i;
            }
            // If i is both maxNode and minNode, then it's root, 
            // so impossible to reach here. Just update maxNode
            maxNode = BSTree<T>::max(this->root);
         }
      }
      else
      {
         if(!i->left && !i->right)
         {
            // if i has no parent, then it's root, which is impossible here
            setParent(i, NULL);
            // the case of i be maxNode has been examined before
            if(minNode == i)
            {
               minNode = BSTree<T>::min(this->root);
            }
#if ADT_PERFORMANCE_ERASE_CONDS
            eraseConds[4]++;
#endif
            delete i;
         }
         else
         {
            if((!i->left && i->right) || (!i->right && i->left)) // single child
            {
               connectSingleChild(i);
               if(i == minNode)
               {
                  minNode = min(this->root);
               }
#if ADT_PERFORMANCE_ERASE_CONDS
               eraseConds[5]++;
#endif
               delete i;
            }
            else
            {
               BSTreeNode<T>* nextOne = BSTree<T>::successor(i);
               if((!nextOne->right && nextOne->left) || (!nextOne->left && nextOne->right))
               {
                  connectSingleChild(nextOne);
                  if(i->parent)
                  {
                     setParent(i, nextOne);
                     nextOne->parent = i->parent;
                  }
                  else
                  {
                     root = nextOne;
                     nextOne->parent = NULL;
                  }
                  nextOne->left = i->left;
                  nextOne->right = i->right;
                  if(i->left)
                  {
                     i->left->parent = nextOne;
                  }
                  if(i->right)
                  {
                     i->right->parent = nextOne;
                  }
#if ADT_PERFORMANCE_ERASE_CONDS
                  eraseConds[6]++;
#endif
                  delete i;
               }
               else
               {
                  // It's impossoble that nextOne->parent == NULL
                  // But nextOne->parent might be i (must be right child in this case)
                  if(nextOne->parent != i)
                  {
                     setParent(nextOne, NULL);
                     nextOne->left = i->left;
                     nextOne->right= i->right;
                     if(nextOne->left)
                     {
                        nextOne->left->parent = nextOne;
                     }
                     if(nextOne->right)
                     {
                        nextOne->right->parent = nextOne;
                     }
                  }
                  else
                  {
                     nextOne->left = i->left;
                     if(nextOne->left)
                     {
                        nextOne->left->parent = nextOne;
                     }
                  }
                  if(i->parent)
                  {
                     setParent(i, nextOne);
                     nextOne->parent = i->parent;
                  }
                  else
                  {
                     root = nextOne;
                     nextOne->parent = NULL;
                  }
                  if(i == minNode) // deleted, so assign the new one
                  {
                     minNode = BSTree<T>::min(this->root);
                  }
#if ADT_PERFORMANCE_ERASE_CONDS
                  eraseConds[7]++;
#endif
                  delete i;
               }
            }
         }
      }
#if ADTP_DEBUG
      try
      {
         verify(this->root);
      }
      catch(pair<BSTreeNode<T>*, const char*>& e)
      {
         cout << "\033[31m" << e.first->data << "->" << e.second << "->parent != " << e.first->data << "\033[0m" << endl;
      }
#endif
      return;
   }
   void clearInternal(BSTreeNode<T>* i)
   {
      // recursive
      if(i->right)
      {
         clearInternal(i->right);
      }
      if(i->left)
      {
         clearInternal(i->left);
      }
      if(!i->left && !i->right) // no children, meaning leaf
      {
         if(!i->parent) // deleting root
         {
            delete i;
            minNode = maxNode = root = NULL;
            return;
         }
         if(i->parent->left == i)
         {
            i->parent->left = NULL;
         }
         if(i->parent->right == i)
         {
            i->parent->right = NULL;
         }
         delete i;
      }
   }
   void printInternal(ostream& os, BSTreeNode<T>* i, size_t indent) const
   {
      // http://stackoverflow.com/questions/1550329/how-can-i-indent-cout-output
      cout << string(indent, ' ');
      if(i)
      {
         os << i->data << "\n";
      }
      else
      {
         os << "[0]\n";
         return; // no more elements to print
      }
      printInternal(os, i->left, indent+2);
      printInternal(os, i->right, indent+2);
      cout.flush();
   }
   void printPtr(const char* name, BSTreeNode<T>* ptr) const
   {
      // default ternary(?:) operator is not defined when two chioces have different type
      cout << name << " = ";
      if(ptr)
      {
         cout << ptr->data;
      }
      else
      {
         cout << "[NULL]";
      }
      cout << endl;
   }
#ifdef ADTP_DEBUG
   void verify(BSTreeNode<T>* i) // for debugging, check connections between nodes
   {
      if(i == NULL) // occurs when called with root == NULL
      {
         return;
      }
      if(i->left)
      {
         if(i->left->parent != i)
         {
            throw make_pair(i, "left");
         }
         verify(i->left);
      }
      if(i->right)
      {
         if(i->right->parent != i)
         {
            throw make_pair(i, "right");
         }
         verify(i->right);
      }
   }
#endif // #if ADTP_DEBUG
   void connectSingleChild(BSTreeNode<T>* i)
   {
      BSTreeNode<T>* target = (i->right)?(i->right):(i->left);
      if(i->parent)
      {
         if(i->parent->left == i)
         {
            i->parent->left = target;
         }
         else if(i->parent->right == i)
         {
            i->parent->right = target;
         }
         target->parent = i->parent;
      }
      else
      {
         root = target;
         target->parent = NULL;
      }
   }
   void setParent(BSTreeNode<T>* i, BSTreeNode<T>* value)
   {
      if(i->parent->left == i)
      {
         i->parent->left = value;
      }
      if(i->parent->right == i)
      {
         i->parent->right = value;
      }
   }
   // data members
   BSTreeNode<T>* root;
   BSTreeNode<T>* minNode; // there are min and max functions
   BSTreeNode<T>* maxNode;
};

#endif // BST_H
