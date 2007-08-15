// Copyright 2007 Georgia Institute of Technology. All rights reserved.
// ABSOLUTELY NOT FOR DISTRIBUTION
/**
 * @file tree/kdtree.h
 *
 * Tools for kd-trees.
 *
 * Eventually we hope to support KD trees with non-L2 (Euclidean)
 * metrics, like Manhattan distance.
 *
 * @experimental
 */

#ifndef TREE_KDTREE_H
#define TREE_KDTREE_H

#include "spacetree.h"
#include "bounds.h"

#include "base/common.h"
#include "col/arraylist.h"
#include "fx/fx.h"

#include "kdtree_impl.h"

/**
 * Regular pointer-style trees (as opposed to THOR trees).
 */
namespace tree {
  /**
   * Creates a KD tree from data, splitting on the midpoint.
   *
   * @experimental
   *
   * This requires you to pass in two unitialized ArrayLists which will contain
   * index mappings so you can account for the re-ordering of the matrix.
   * (By unitialized I mean don't call Init on it)
   *
   * @param matrix data where each column is a point, WHICH WILL BE RE-ORDERED
   * @param leaf_size the maximum points in a leaf
   * @param old_from_new pointer to an unitialized arraylist; it will map
   *        new indices to original
   * @param new_from_old pointer to an unitialized arraylist; it will map
   *        original indexes to new indices
   */
  template<typename TKdTree>
  TKdTree *MakeKdTreeMidpoint(Matrix& matrix, index_t leaf_size,
      ArrayList<index_t> *old_from_new = NULL,
      ArrayList<index_t> *new_from_old = NULL) {
    TKdTree *node = new TKdTree();
    index_t *old_from_new_ptr;

    if (old_from_new) {
      old_from_new->Init(matrix.n_cols());
      
      for (index_t i = 0; i < matrix.n_cols(); i++) {
        (*old_from_new)[i] = i;
      }
      
      old_from_new_ptr = old_from_new->begin();
    } else {
      old_from_new_ptr = NULL;
    }
      
    node->Init(0, matrix.n_cols());
    node->bound().Init(matrix.n_rows());
    tree_kdtree_private::FindBoundFromMatrix(matrix,
        0, matrix.n_cols(), &node->bound());

    tree_kdtree_private::SplitKdTreeMidpoint(matrix, node, leaf_size,
        old_from_new_ptr);
    
    if (new_from_old) {
      new_from_old->Init(matrix.n_cols());
      for (index_t i = 0; i < matrix.n_cols(); i++) {
        (*new_from_old)[(*old_from_new)[i]] = i;
      }
    }
    
    return node;
  }

  /**
   * Loads a KD tree from a command-line parameter,
   * creating a KD tree if necessary.
   *
   * @experimental
   *
   * This optionally allows the end user to write out the created KD tree
   * to a file, as a convenience.
   *
   * Requires a sub-module, with the root parameter of the submodule being
   * the filename, and optional parameters leaflen, type, and save (see
   * example below).
   *
   * Example:
   *
   * @code
   * MyKdTree *q_tree;
   * Matrix q_matrix;
   * ArrayList<index_t> q_permutation;
   * LoadKdTree(fx_submodule(NULL, "q", "q"), &q_matrix, &q_tree,
   *    &q_permutation);
   * @endcode
   *
   * Command-line use:
   *
   * @code
   * ./main --q=foo.txt                  # load from csv format
   * ./main --q=foo.txt --q/leaflen=20   # leaf length
   * @endcode
   *
   * @param module the module to get parameters from
   * @param matrix the matrix to initialize, undefined on failure
   * @param tree_pp an unitialized pointer that will be set to the root
   *        of the tree, must still be freed on failure
   * @param old_from_new stores the permutation to get from the indices in
   *        the matrix returned to the original data point indices
   * @return SUCCESS_PASS or SUCCESS_FAIL
   */
  template<typename TKdTree>
  success_t LoadKdTree(datanode *module,
      Matrix *matrix, TKdTree **tree_pp,
      ArrayList<index_t> *old_from_new) {
    const char *type = fx_param_str(module, "type", "text");
    const char *fname = fx_param_str(module, "", NULL);
    success_t success = SUCCESS_PASS;

    fx_timer_start(module, "load");
    if (strcmp(type, "text") == 0) {
      int leaflen = fx_param_int(module, "leaflen", 20);

      fx_timer_start(module, "load_matrix");
      success = data::Load(fname, matrix);
      fx_timer_stop(module, "load_matrix");

      //if (fx_param_exists("do_pca")) {}

      fx_timer_start(module, "make_tree");
      *tree_pp = MakeKdTreeMidpoint<TKdTree>(
          *matrix, leaflen, old_from_new);
      fx_timer_stop(module, "make_tree");
    }
    fx_timer_stop(module, "load");

    return success;
  }
};

/** Basic KD tree structure. @experimental */
typedef BinarySpaceTree<DHrectBound<2>, Matrix> BasicKdTree;

#endif
