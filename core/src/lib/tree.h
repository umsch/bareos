/*
   BAREOS® - Backup Archiving REcovery Open Sourced

   Copyright (C) 2002-2009 Free Software Foundation Europe e.V.
   Copyright (C) 2016-2024 Bareos GmbH & Co. KG

   This program is Free Software; you can redistribute it and/or
   modify it under the terms of version three of the GNU Affero General Public
   License as published by the Free Software Foundation and included
   in the file LICENSE.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/
// Kern Sibbald, June MMII
/**
 * @file
 * Directory tree build/traverse routines
 */

#ifndef BAREOS_LIB_TREE_H_
#define BAREOS_LIB_TREE_H_

#include <vector>
#include "include/baconfig.h"
#include "lib/htable.h"
#include "lib/rblist.h"

#include "include/config.h"

struct s_mem {
  struct s_mem* next; /* next buffer */
  int rem;            /* remaining bytes */
  void* mem;          /* memory pointer */
  char first[1];      /* first byte */
};

#define USE_DLIST

#define foreach_child(var, list) \
  for ((var) = NULL;             \
       (*((TREE_NODE**)&(var)) = (TREE_NODE*)(list->child.next(var)));)

#define TreeNodeHasChild(node) ((node)->child.size() > 0)

#define first_child(node) \
        ((TREE_NODE *)(node->child.first())

struct delta_list {
  struct delta_list* next;
  JobId_t JobId;
  int32_t FileIndex;
};

/**
 * Keep this node as small as possible because
 *   there is one for each file.
 */
struct s_tree_node : rblink {
  s_tree_node()
      : type{false}
      , extract{false}
      , extract_dir{false}
      , hard_link{false}
      , soft_link{false}
      , inserted{false}
      , loaded{false}
      , in_use{false}
  {
  }

  /* KEEP sibling as the first member to avoid having to
   *  do initialization of child */

  // rblink has alignment of 8 but ends with a single char at the end,
  // so we have 7 bytes (with alignment = 1) that we can use fill
  // up with our own data for free

  unsigned int type : 8;        /* node type */
  unsigned int extract : 1;     /* extract item */
  unsigned int extract_dir : 1; /* extract dir entry only */
  unsigned int hard_link : 1;   /* set if have hard link */
  unsigned int soft_link : 1;   /* set if is soft link */
  unsigned int inserted : 1;    /* set when node newly inserted */
  unsigned int loaded : 1;      /* set when the dir is in the tree */
  unsigned int in_use : 1;      /* set when the dir is in the tree */
  uint32_t JobId{};             /* JobId */

  struct s_tree_node* parent{};
  struct s_tree_node* next{};      /* next hash of FileIndex */
  struct delta_list* delta_list{}; /* delta parts for this node */
  char* fname{};                   /* file name */
  uint64_t fhinfo{};               /* NDMP Fh_info */
  uint64_t fhnode{};               /* NDMP Fh_node */

  rblist child;
  int32_t FileIndex{}; /* file index */
  int32_t delta_seq{}; /* current delta sequence */
};
typedef struct s_tree_node TREE_NODE;


struct node_allocator {
  s_tree_node* allocate();
  void free(s_tree_node* n);

  node_allocator()
  {
    pages.emplace_back(std::make_unique<s_tree_node[]>(start_count));
  }

  size_t indexof(s_tree_node* n);
  s_tree_node* get(size_t index);

 private:
  s_tree_node* freelist;

  static constexpr size_t start_count = 1024;

  // this is an index into the current array
  size_t next_slot{0};

  // the pages have sizes:
  // X << 0, X << 1, X << 2, ...
  // where X = start_count
  std::vector<std::unique_ptr<s_tree_node[]>> pages;

  static constexpr size_t page_size(size_t index)
  {
    size_t doubling_factor = index;
    return start_count << doubling_factor;
  }

  static constexpr size_t cum_page_size_until(size_t index)
  {
    // computes the cumulative page size of page indices < index
    if (index == 0) { return 0; }

    return start_count << (index - 1);
  }
};

/* hardlink hashtable entry */
struct s_hl_entry {
  uint64_t key;
  hlink link;
  TREE_NODE* node;
};
typedef struct s_hl_entry HL_ENTRY;

using HardlinkTable = htable<uint64_t, HL_ENTRY, MonotonicBuffer::Size::Small>;

struct s_tree_root : public s_tree_node {
  struct s_tree_node* first{}; /* first entry in the tree */
  struct s_tree_node* last{};  /* last entry in tree */
  struct s_mem* mem{};         /* tree memory */
  uint32_t total_size{};       /* total bytes allocated */
  uint32_t blocks{};           /* total mallocs */
  int cached_path_len{};       /* length of cached path */
  char* cached_path{};         /* cached current path */
  TREE_NODE* cached_parent{};  /* cached parent for above path */
  HardlinkTable hardlinks;     /* references to first occurence of hardlinks */

  size_t current_node_id{};
  node_allocator alloc;
};
typedef struct s_tree_root TREE_ROOT;

/* type values */
#define TN_ROOT 1    /* root node */
#define TN_NEWDIR 2  /* created directory to fill path */
#define TN_DIR 3     /* directory entry */
#define TN_DIR_NLS 4 /* directory -- no leading slash -- win32 */
#define TN_FILE 5    /* file entry */

/* External interface */
TREE_ROOT* new_tree(int count);
TREE_NODE* insert_tree_node(char* path,
                            char* fname,
                            int type,
                            TREE_ROOT* root,
                            TREE_NODE* parent);
TREE_NODE* make_tree_path(char* path, TREE_ROOT* root);
TREE_NODE* tree_cwd(char* path, TREE_ROOT* root, TREE_NODE* node);
TREE_NODE* tree_relcwd(char* path, TREE_ROOT* root, TREE_NODE* node);
void TreeAddDeltaPart(TREE_ROOT* root,
                      TREE_NODE* node,
                      JobId_t JobId,
                      int32_t FileIndex);
void FreeTree(TREE_ROOT* root);
POOLMEM* tree_getpath(TREE_NODE* node);
void TreeRemoveNode(TREE_ROOT* root, TREE_NODE* node);

/**
 * Use the following for traversing the whole tree. It will be
 *   traversed in the order the entries were inserted into the
 *   tree.
 */
#define FirstTreeNode(r) (r)->first
#define NextTreeNode(n) (n)->next


size_t NodeIndex(TREE_ROOT* root, TREE_NODE* node);
s_tree_node* NodeWithIndex(TREE_ROOT* root, size_t index);

#endif  // BAREOS_LIB_TREE_H_
