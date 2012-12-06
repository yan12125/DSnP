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
#define ADT_PERFORMANCE 0

#if ADT_PERFORMANCE
   clock_t clocks[10];
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
#if ADT_PERFORMANCE
      for(int i=0;i<10;i++)
      {
         cout << (1.0*clocks[i])/CLOCKS_PER_SEC << " ";
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
      for(BSTreeNode<T>* cur = minNode;cur != NULL;cur = BSTree<T>::successor(cur))
      {
         if(cur->data == i)
         {
            eraseInternal(cur);
            return true;
         }
      }
      return false;
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
      cout << "root = ";
      if(root)
      {
         cout << root->data;
      }
      else
      {
         cout << "[NULL]";
      }
      cout << "\nminNode = ";
      if(minNode)
      {
         cout << minNode->data;
      }
      else
      {
         cout << "[NULL]" ;
      }
      cout << "\nmaxNode = ";
      if(maxNode)
      {
         cout << maxNode->data;
      }
      else
      {
         cout << "[NULL]";
      }
      cout << endl;
#endif // #if ADTP_DEBUG      
   }
private:
   // helper functions
   static BSTreeNode<T>* successor(BSTreeNode<T>* i)
   {
#if ADT_PERFORMANCE
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
            BSTreeNode<T>* candidate1 = i->parent;
            BSTreeNode<T>* candidate2 = BSTree<T>::min(i->right);
            ret = (candidate1->data < candidate2->data)?candidate1:candidate2;
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
#if ADT_PERFORMANCE
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
            BSTreeNode<T>* candidate1 = i->parent;
            BSTreeNode<T>* candidate2 = BSTree<T>::max(i->left);
            return (candidate2->data < candidate1->data)?candidate1:candidate2;
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
#if ADT_PERFORMANCE
      clock_t now = clock();
#endif
      BSTreeNode<T>* cur = i;
      while(cur->left)
      {
         cur = cur->left;
      }
#if ADT_PERFORMANCE
      clocks[0] += clock() - now;
#endif
      return cur;
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
               delete i;
            }
            else
            {
               root = maxNode = minNode = NULL;
               delete i;
            }
         }
         else
         {
            if(i->left)
            {
               i->parent->right = i->left;
               i->left->parent = i->parent;
               delete i;
            }
            else
            {
               i->parent->right = NULL;
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
            if(i->parent->left == i)
            {
               i->parent->left = NULL;
            }
            if(i->parent->right == i)
            {
               i->parent->right = NULL;
            }
            // the case of i be maxNode has been examined before
            if(minNode == i)
            {
               minNode = BSTree<T>::min(this->root);
            }
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
                     if(i->parent->left == i)
                     {
                        i->parent->left = nextOne;
                     }
                     if(i->parent->right == i)
                     {
                        i->parent->right = nextOne;
                     }
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
                  delete i;
               }
               else
               {
                  if(!nextOne->left && !nextOne->right) // successor is a leaf
                  {
                     // It's impossoble that nextOne->parent == NULL
                     // But nextOne->parent might be i (must be right child in this case)
                     if(nextOne->parent != i)
                     {
                        if(nextOne->parent->left == nextOne)
                        {
                           nextOne->parent->left = NULL;
                        }
                        if(nextOne->parent->right == nextOne)
                        {
                           nextOne->parent->right = NULL;
                        }
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
                        if(i->parent->left == i)
                        {
                           i->parent->left = nextOne;
                        }
                        if(i->parent->right == i)
                        {
                           i->parent->right = nextOne;
                        }
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
                     delete i;
                  }
               }
            }
         }
      }
      try
      {
         verify(this->root);
      }
      catch(pair<BSTreeNode<T>*, const char*>& e)
      {
         cout << "\033[31m" << e.first->data << "->" << e.second << "->parent != " << e.first->data << "\033[0m" << endl;
      }
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
   // data members
   BSTreeNode<T>* root;
   BSTreeNode<T>* minNode; // there are min and max functions
   BSTreeNode<T>* maxNode;
};

#endif // BST_H
