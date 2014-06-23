/**
 * @file rectangle_tree_impl.hpp
 * @author Andrew Wells
 *
 * Implementation of generalized rectangle tree.
 */
#ifndef __MLPACK_CORE_TREE_RECTANGLE_TREE_RECTANGLE_TREE_IMPL_HPP
#define __MLPACK_CORE_TREE_RECTANGLE_TREE_RECTANGLE_TREE_IMPL_HPP

// In case it wasn't included already for some reason.
#include "rectangle_tree.hpp"

#include <mlpack/core/util/cli.hpp>
#include <mlpack/core/util/log.hpp>
#include <mlpack/core/util/string_util.hpp>

namespace mlpack {
namespace tree {

template<typename SplitType,
         typename DescentType,
	 typename StatisticType,
         typename MatType>
RectangleTree<SplitType, DescentType, StatisticType, MatType>::RectangleTree(
    MatType& data,
    const size_t maxLeafSize = 20,
    const size_t minLeafSize = 6,
    const size_t maxNumChildren = 4,
    const size_t minNumChildren = 0,
    const size_t firstDataIndex = 0):
    maxNumChildren(maxNumChildren),
    minNumChildren(minNumChildren),
    numChildren(0),
    children(maxNumChildren+1), // Add one to make splitting the node simpler
    parent(NULL),
    begin(0),
    count(0),
    maxLeafSize(maxLeafSize),
    minLeafSize(minLeafSize),
    bound(data.n_rows),
    parentDistance(0),
    dataset(new MatType(data.n_rows, static_cast<int>(maxLeafSize)+1)) // Add one to make splitting the node simpler
{
  stat = StatisticType(*this);
  
  std::cout << ToString() << std::endl;
  
  
  // For now, just insert the points in order.
  RectangleTree* root = this;
  for(int i = firstDataIndex; i < 54 /*data.n_cols*/; i++) {
    std::cout << "inserting point number: " << i << std::endl;
    root->InsertPoint(data.col(i));
    std::cout << "finished inserting point number: " << i << std::endl;
    if(root->Parent() != NULL) {
      root = root->Parent(); // OK since the level increases by at most one per iteration.
    }
    std::cout << "hi" << std::endl;
    std::cout << ToString() << std::endl;
  }
  
}

template<typename SplitType,
	 typename DescentType,
	 typename StatisticType,
	 typename MatType>
RectangleTree<SplitType, DescentType, StatisticType, MatType>::RectangleTree(
  RectangleTree<SplitType, DescentType, StatisticType, MatType>* parentNode):
  maxNumChildren(parentNode->MaxNumChildren()),
  minNumChildren(parentNode->MinNumChildren()),
  numChildren(0),
  children(maxNumChildren+1),
  parent(parentNode),
  begin(0),
  count(0),
  maxLeafSize(parentNode->MaxLeafSize()),
  minLeafSize(parentNode->MinLeafSize()),
  bound(parentNode->Bound().Dim()),
  parentDistance(0),
  dataset(new MatType(static_cast<int>(parentNode->Bound().Dim()), static_cast<int>(maxLeafSize)+1)) // Add one to make splitting the node simpler
{
  stat = StatisticType(*this);
}

/**
 * Deletes this node, deallocating the memory for the children and calling
 * their destructors in turn.  This will invalidate any pointers or references
 * to any nodes which are children of this one.
 */
template<typename SplitType,
         typename DescentType,
	typename StatisticType,
         typename MatType>
RectangleTree<SplitType, DescentType, StatisticType, MatType>::
  ~RectangleTree()
{
  for(int i = 0; i < numChildren; i++) {
    delete children[i];
  }
  delete dataset;
}

/**
  * Deletes this node but leaves the children untouched.  Needed for when we 
  * split nodes and remove nodes (inserting and deleting points).
  */
template<typename SplitType,
                  typename DescentType,
                  typename StatisticType,
                  typename MatType>
void RectangleTree<SplitType, DescentType, StatisticType, MatType>::
    softDelete()
{
  /* do nothing.  I'm not sure how to handle this yet, so for now, we will leak memory */
}

/**
 * Recurse through the tree and insert the point at the leaf node chosen
 * by the heuristic.
 */
template<typename SplitType,
         typename DescentType,
	 typename StatisticType,
         typename MatType>
void RectangleTree<SplitType, DescentType, StatisticType, MatType>::
    InsertPoint(const arma::vec& point)
{
  
  std::cout << "insert point called" << std::endl;
  // Expand the bound regardless of whether it is a leaf node.
  bound |= point;

  // If this is a leaf node, we stop here and add the point.
  if(numChildren == 0) {
    std::cout << "count = " << count << std::endl;
    dataset->col(count++) = point;
    SplitNode();
    return;
  }

  // If it is not a leaf node, we use the DescentHeuristic to choose a child
  // to which we recurse.
  double minScore = DescentType::EvalNode(children[0]->Bound(), point);
  int bestIndex = 0;
  
  for(int i = 1; i < numChildren; i++) {
    double score = DescentType::EvalNode(children[i]->Bound(), point);
    if(score < minScore) {
      minScore = score;
      bestIndex = i;
    }
  }
  children[bestIndex]->InsertPoint(point);
}

template<typename SplitType,
         typename DescentType,
	 typename StatisticType,
         typename MatType>
size_t RectangleTree<SplitType, DescentType, StatisticType, MatType>::
    TreeSize() const
{
  int n = 0;
  for(int i = 0; i < numChildren; i++) {
    n += children[i]->TreeSize();
  }
  return n + 1; // we add one for this node
}



template<typename SplitType,
         typename DescentType,
	 typename StatisticType,
         typename MatType>
size_t RectangleTree<SplitType, DescentType, StatisticType, MatType>::
    TreeDepth() const
{
  /* Because R trees are balanced, we could simplify this.  However, X trees are not 
     guaranteed to be balanced so I keep it as is: */

  // Recursively count the depth of each subtree.  The plus one is
  // because we have to count this node, too.
  int maxSubDepth = 0;
  for(int i = 0; i < numChildren; i++) {
    int d = children[i]->TreeDepth();
    if(d > maxSubDepth)
      maxSubDepth = d;
  }
  return maxSubDepth + 1;
}

template<typename SplitType,
         typename DescentType,
	 typename StatisticType,
         typename MatType>
inline bool RectangleTree<SplitType, DescentType, StatisticType, MatType>::
    IsLeaf() const
{
  return numChildren == 0;
}


/**
 * Return a bound on the furthest point in the node form the centroid.
 * This returns 0 unless the node is a leaf.
 */
template<typename SplitType,
         typename DescentType,
	 typename StatisticType,
         typename MatType>
inline double RectangleTree<SplitType, DescentType, StatisticType, MatType>::
FurthestPointDistance() const
{
  if(!IsLeaf())
    return 0.0;

  // Otherwise return the distance from the centroid to a corner of the bound.
  return 0.5 * bound.Diameter();
}

/**
 * Return the furthest possible descendant distance.  This returns the maximum
 * distance from the centroid to the edge of the bound and not the empirical
 * quantity which is the actual furthest descendant distance.  So the actual
 * furthest descendant distance may be less than what this method returns (but
 * it will never be greater than this).
 */
template<typename SplitType,
         typename DescentType,
	 typename StatisticType,
         typename MatType>
inline double RectangleTree<SplitType, DescentType, StatisticType, MatType>::
    FurthestDescendantDistance() const
{
  return furthestDescendantDistance;
}

/**
 * Return the number of points contained in this node.  Zero if it is a non-leaf node.
 */
template<typename SplitType,
	 typename DescentType,
	 typename StatisticType,
	 typename MatType>
inline size_t RectangleTree<SplitType, DescentType, StatisticType, MatType>::
    NumPoints() const
{
  if(numChildren != 0) // This is not a leaf node.
    return 0;

  return count;
}

/**
 * Return the number of descendants contained in this node.  MEANINIGLESS AS IT CURRENTLY STANDS.
 * USE NumPoints() INSTEAD.
 */
template<typename SplitType,
	 typename DescentType,
	 typename StatisticType,
	 typename MatType>
inline size_t RectangleTree<SplitType, DescentType, StatisticType, MatType>::
    NumDescendants() const
{
  return count;
}

/**
 * Return the index of a particular descendant contained in this node.  SEE OTHER WARNINGS
 */
template<typename SplitType,
	 typename DescentType,
	 typename StatisticType,
	 typename MatType>
inline size_t RectangleTree<SplitType, DescentType, StatisticType, MatType>::
    Descendant(const size_t index) const
{
  return (begin + index);
}

/**
 * Return the index of a particular point contained in this node.  SEE OTHER WARNINGS
 */
template<typename SplitType,
	 typename DescentType,
	 typename StatisticType,
	 typename MatType>
inline size_t RectangleTree<SplitType, DescentType, StatisticType, MatType>::
    Point(const size_t index) const
{
  return (begin + index);
}

/**
 * Return the last point in the tree.  SINCE THE TREE STORES DATA SEPARATELY IN EACH LEAF
 * THIS IS CURRENTLY MEANINGLESS.
 */
template<typename SplitType,
	 typename DescentType,
	 typename StatisticType,
	 typename MatType>
inline size_t RectangleTree<SplitType, DescentType, StatisticType, MatType>::End() const
{
  if(numChildren)
    return begin + count;
  return children[numChildren-1]->End();
}

  //have functions for returning the list of modified indices if we end up doing it that way.

/**
 * Split the tree.  This calls the SplitType code to split a node.  This method should only
 * be called on a leaf node.
 */
template<typename SplitType,
	 typename DescentType,
	 typename StatisticType,
	 typename MatType>
void RectangleTree<SplitType, DescentType, StatisticType, MatType>::SplitNode()
{
  // This should always be a leaf node.  When we need to split other nodes,
  // the split will be called from here but will take place in the SplitType code.
  assert(numChildren == 0);

  // Check to see if we are full.
  if(count < maxLeafSize)
    return; // We don't need to split.
  
  std::cout << "we are actually splitting the node." << std::endl;
  // If we are full, then we need to split (or at least try).  The SplitType takes
  // care of this and of moving up the tree if necessary.
  SplitType::SplitLeafNode(this);
  std::cout << "we finished actually splitting the node." << std::endl;
  
  std::cout << ToString() << std::endl;
}


/**
 * Returns a string representation of this object.
 */
template<typename SplitType,
	 typename DescentType,
	 typename StatisticType,
	 typename MatType>
std::string RectangleTree<SplitType, DescentType, StatisticType, MatType>::ToString() const
{
  std::ostringstream convert;
  convert << "RectangleTree [" << this << "]" << std::endl;
  convert << "  First point: " << begin << std::endl;
  convert << "  Number of descendants: " << numChildren << std::endl;
  convert << "  Number of points: " << count << std::endl;
  convert << "  Bound: " << std::endl;
  convert << mlpack::util::Indent(bound.ToString(), 2);
  convert << "  Statistic: " << std::endl;
  //convert << mlpack::util::Indent(stat.ToString(), 2);
  convert << "  Max leaf size: " << maxLeafSize << std::endl;
  convert << "  Min leaf size: " << minLeafSize << std::endl;
  convert << "  Max num of children: " << maxNumChildren << std::endl;
  convert << "  Min num of children: " << minNumChildren << std::endl;
  convert << "  Parent address: " << parent << std::endl;

  // How many levels should we print?  This will print the root and it's children.
  if(parent == NULL) {
    for(int i = 0; i < numChildren; i++) {
      convert << children[i]->ToString();
    }
  }
  return convert.str();
}

}; //namespace tree
}; //namespace mlpack

#endif