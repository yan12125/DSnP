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
      iterator(BSTreeNode<T>* data): node(data)
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
         this->node = BSTree<T>::predecessor(this->node);
         return *this;
      }
      iterator operator--(int)
      {
         iterator temp = *this;
         this->node = BSTree<T>::predecessor(this->node);
         return *this;
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

      friend class BSTree<T>;
   };

   iterator begin() const
   {
      return iterator(this->minNode);
   }
   iterator end() const
   {
      return iterator(NULL);
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
      eraseInternal(minNode);
   }
   void pop_back()
   {
      eraseInternal(maxNode);
   }
   bool erase(const T& i)
   {
      for(BSTreeNode<T>* cur = minNode;cur != NULL;cur = BSTree<T>::successor(cur))
      {
         if(cur->data == i)
         {
            eraseInternal(cur);
         }
      }
      return false;
   }
   bool erase(iterator pos)
   {
      eraseInternal(pos.node);
      return false;
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
   }
private:
   // helper functions
   static BSTreeNode<T>* successor(BSTreeNode<T>* i)
   {
      if(!i->parent) // root
      {
         return (i->right)?(BSTree<T>::min(i->right)):(NULL);
      }
      if(i->parent->left == i)
      {
         if(i->right)
         {
            BSTreeNode<T>* candidate1 = i->parent;
            BSTreeNode<T>* candidate2 = BSTree<T>::min(i->right);
            return (candidate1->data < candidate2->data)?candidate1:candidate2;
         }
         else
         {
            return i->parent;
         }
      }
      // parent's right child
      if(i->right)
      {
         return BSTree<T>::min(i->right);
      }
      // successor are from upper levels
      BSTreeNode<T>* cur = i;
      while(1)
      {
         if(!cur->parent) // come to root, means no successor
         {
            return NULL;
         }
         if(cur->parent->left == cur)
         {
            return cur->parent;
         }
         cur = cur->parent;
      }
   }
   static BSTreeNode<T>* predecessor(BSTreeNode<T>* i)
   {
      if(!i->parent) // root
      {
         return (i->left)?(BSTree<T>::min(i->left)):(NULL);
      }
      if(i->parent->right == i)
      {
         if(i->left)
         {
            BSTreeNode<T>* candidate1 = i->parent;
            BSTreeNode<T>* candidate2 = BSTree<T>::min(i->left);
            return (candidate1->data < candidate2->data)?candidate1:candidate2;
         }
         else
         {
            return i->parent;
         }
      }
      // parent's right child
      if(i->left)
      {
         return BSTree<T>::min(i->left);
      }
      // successor are from upper levels
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
      BSTreeNode<T>* cur = i;
      while(1)
      {
         if(!cur->left)
         {
            return cur;
         }
         cur = cur->left;
      }
   }
   static BSTreeNode<T>* max(BSTreeNode<T>* i)
   {
      BSTreeNode<T>* cur = i;
      while(1)
      {
         if(!cur->right)
         {
            return cur;
         }
         cur = cur->right;
      }
   }
   size_t sizeInternal(BSTreeNode<T>* i) const
   {
      // recursive
      if(!i->left && !i->right)
      {
         return 1;
      }
      // else
      return ( (i->left ?sizeInternal(i->left): 0) +
               (i->right?sizeInternal(i->right):0) );
   }
   void eraseInternal(BSTreeNode<T>* i)
   {
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
   // data members
   BSTreeNode<T>* root;
   BSTreeNode<T>* minNode; // there are min and max functions
   BSTreeNode<T>* maxNode;
};

#endif // BST_H
