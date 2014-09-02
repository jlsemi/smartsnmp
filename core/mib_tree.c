/*
 * This file is part of SmartSNMP
 * Copyright (C) 2014, Credo Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mib.h"
#include "snmp.h"

/* Dummy root node */
static struct mib_group_node mib_dummy_node = {
  MIB_OBJ_GROUP,
  1,
  0,
  NULL,
  NULL
};

static oid_t *root_oid;
static uint32_t root_oid_len;

oid_t *
oid_dup(const oid_t *oid, uint32_t len)
{
  int i;
  oid_t *new_oid;
  /* We need to allocate largest space so as to hold any oid */
  new_oid = xmalloc(MIB_VALUE_MAX_LEN * sizeof(oid_t));
  for (i = 0; i < len; i++) {
    new_oid[i] = oid[i];
  }
  return new_oid;
}

int
oid_cmp(const oid_t *src, uint32_t src_len, const oid_t *target, uint32_t tar_len)
{
  int ret;

  while (tar_len-- && src_len-- && !(ret = (int)(*src++ - *target++)))
    continue;

  if (!ret)
    return src_len;
  else
    return ret;
}

oid_t *
oid_cpy(oid_t *oid_dest, const oid_t *oid_src, uint32_t len)
{
  oid_t *dest = oid_dest;

  while (len-- > 0) {
    *dest++ = *oid_src++;
  }

  return oid_dest;
}

static int
oid_binary_search(oid_t *arr, int n, oid_t oid)
{
  int low = -1;
  int high = n;

  /* assert(n >= 0); */
  /* Search the last position that fits target. */
  while (low + 1 < high) {
    int mid = low + (high - low) / 2;
    if (arr[mid] > oid)
      high = mid;
    else
      low = mid;
  }

  if (low < 0 || arr[low] != oid)
    return -low - 2;
  else
    return low;  /* low == high - 1 */
}

/* GET request search, depth-first traversal in mib-tree, oid must match */
struct mib_node *
mib_tree_search(const oid_t *orig_oid, uint32_t orig_id_len, struct oid_search_res *ret_oid)
{
  struct mib_node *node;
  struct mib_group_node *gn;
  struct mib_instance_node *in;
  oid_t *oid;
  uint32_t id_len;

  /* Duplicate OID as return value */
  ret_oid->oid = oid_dup(orig_oid, orig_id_len);
  ret_oid->id_len = orig_id_len;
  ret_oid->exist_state = 0;

  /* Init something */
  node = (struct mib_node *)&mib_dummy_node;
  oid = ret_oid->oid;
  id_len = ret_oid->id_len;

  while (node != NULL && id_len > 0) {
    switch (node->type) {

      case MIB_OBJ_GROUP:
        gn = (struct mib_group_node *)node;
        int i = oid_binary_search(gn->sub_id, gn->sub_id_cnt, *oid);
        if (i >= 0) {
          /* Sub-id found, go on loop */
          oid++;
          id_len--;
          node = gn->sub_ptr[i];
          continue;
        } else {
          /* Sub-id not found */
          ret_oid->inst_id = oid;
          ret_oid->inst_id_len = id_len;
          ret_oid->exist_state = BER_TAG_NO_SUCH_OBJ;
          return node;
        }

      case MIB_OBJ_INSTANCE:
        in = (struct mib_instance_node *)node;
        /* Find instance variable through lua handler function */
        ret_oid->inst_id = oid;
        ret_oid->inst_id_len = id_len;
        ret_oid->callback = in->callback;
        ret_oid->exist_state = mib_instance_search(ret_oid);
        return node;

      default:
        assert(0);
    }
  }

  /* node == NULL || id_len == 0 */
  ret_oid->inst_id = oid;
  ret_oid->inst_id_len = id_len;
  if (node && node->type == MIB_OBJ_INSTANCE) {
    ret_oid->exist_state = BER_TAG_NO_SUCH_INST;
  } else {
    ret_oid->exist_state = BER_TAG_NO_SUCH_OBJ;
  }
  return node;
}

struct node_backlog {
  /* node to be backlogged */
  struct mib_node *node;
  /* next sub-id index of the node */
  int n_idx;
};

static inline void
nbl_push(const struct node_backlog *nbl, struct node_backlog **top, struct node_backlog **buttom)
{
  if (*top - *buttom < MIB_OID_MAX_LEN) {
    *((*top)++) = *nbl;
  }
}

static inline struct node_backlog *
nbl_pop(struct node_backlog **top, struct node_backlog **buttom)
{
  if (*top > *buttom) {
    return --*top;
  } else {
    return NULL;
  }
}

/* GETNEXT request search, depth-first traversal in mib-tree, find the closest next oid. */
struct mib_node *
mib_tree_search_next(const oid_t *orig_oid, uint32_t orig_id_len, struct oid_search_res *ret_oid)
{
  oid_t *oid;
  uint32_t id_len;
  uint8_t immediate; /* This is the search state indicator */

  struct node_backlog nbl, *p_nbl;
  struct node_backlog nbl_stk[MIB_OID_MAX_LEN];
  struct node_backlog *stk_top, *stk_buttom;

  struct mib_node *node;
  struct mib_group_node *gn;
  struct mib_instance_node *in;

  /* Check dummy root oid prefix */
  if (orig_id_len > root_oid_len) {
    immediate = 0;  /* 0 is to search the first match node in mib tree. */
    ret_oid->oid = oid_dup(orig_oid, orig_id_len);
    ret_oid->id_len = orig_id_len;
    if (oid_cmp(orig_oid, root_oid_len, root_oid, root_oid_len) > 0) {
      /* END_OF_MIB_VIEW */
      node = NULL;
    } else {
      node = (struct mib_node *)&mib_dummy_node;
    }
  } else {
    immediate = 1;  /* 1 is to get the immediate closest instance */
    ret_oid->oid = oid_dup(root_oid, root_oid_len);
    ret_oid->id_len = root_oid_len;
    if (oid_cmp(orig_oid, orig_id_len, root_oid, root_oid_len) > 0) {
      /* END_OF_MIB_VIEW */
      node = NULL;
    } else {
      node = (struct mib_node *)&mib_dummy_node;
    }
  }

  /* Init something */
  p_nbl = NULL;
  stk_top = stk_buttom = nbl_stk;
  oid = ret_oid->oid;
  id_len = ret_oid->id_len;
  ret_oid->inst_id = NULL;
  ret_oid->inst_id_len = 0;
  ret_oid->exist_state = 0;

  for (; ;) {

    if (node != NULL)
    switch (node->type) {

      case MIB_OBJ_GROUP:
        gn = (struct mib_group_node *)node;

        if (immediate) {
          /* Fetch the immediate instance node. */
          int i;

          if (p_nbl != NULL) {
            /* Fetch the sub-id next to the pop-up backlogged one. */
            i = p_nbl->n_idx;
            p_nbl = NULL;
          } else {
            /* Fetch the first sub-id. */
            i = 0;
          }

          if (i + 1 >= gn->sub_id_cnt) {
            /* Last sub-id, mark NULL and -1. */
            nbl.node = NULL;
            nbl.n_idx = -1;
          } else {
            nbl.node = node;
            nbl.n_idx = i + 1;
          }
          /* Backlog the current node and move on. */
          nbl_push(&nbl, &stk_top, &stk_buttom);
          *oid++ = gn->sub_id[i];
          node = gn->sub_ptr[i];
        } else {
          /* Search the match sub-id */
          int index = oid_binary_search(gn->sub_id, gn->sub_id_cnt, *oid);
          int i = index;

          if (index < 0) {
            /* Not found, switch to the immediate search mode */
            immediate = 1;

            /* Reverse the sign to locate the right position. */
            i = -i - 1;
            if (i == gn->sub_id_cnt) {
              /* All sub-ids are greater than the target;
               * Backtrack and fetch the next one. */
              break;
            } else if (i == 0) {
              /* 1. All sub-ids are less than the target;
               * 2. No sub-id in this group node;
               * Just switch to the immediate search mode */
              continue;
            } /* else {
              Target is between the two sub-ids and [i] is the next one,
              switch to immediate mode and move on.
            } */
          }

          /* Sub-id found is greater or just equal to the target,
           * Anyway, record the next node and push it into stack. */
          if (i + 1 >= gn->sub_id_cnt) {
            /* Last sub-id, mark NULL and -1. */
            nbl.node = NULL;
            nbl.n_idx = -1;
          } else {
            nbl.node = node;
            nbl.n_idx = i + 1;
          }

          /* Backlog the current node and move on. */
          nbl_push(&nbl, &stk_top, &stk_buttom);
          *oid++ = gn->sub_id[i];
          node = gn->sub_ptr[i];
          if (--id_len == 0 && node->type == MIB_OBJ_GROUP) {
            /* When oid length is decreased to zero, switch to the immediate mode */
            immediate = 1;
          }
        }

        continue; /* Go on loop */

      case MIB_OBJ_INSTANCE:
        in = (struct mib_instance_node *)node;

        if (immediate || id_len == 0) {
          /* Fetch the first instance variable */
          ret_oid->inst_id_len = 0;
        } else {
          /* Search the closest instance whose oid is greater than the target */
          ret_oid->inst_id_len = id_len;
        }

        /* Find instance variable through lua handler function */
        ret_oid->inst_id = oid;
        ret_oid->callback = in->callback;
        ret_oid->exist_state = mib_instance_search(ret_oid);
        if (ret_oid->exist_state == 0) {
          ret_oid->id_len = oid - ret_oid->oid + ret_oid->inst_id_len;
          assert(ret_oid->id_len <= MIB_OID_MAX_LEN);
          return node;
        }

        break;  /* Instance not found */

      default:
        assert(0);
    }

    /* Backtracking condition:
     * 1. No greater sub-id in group node;
     * 2. Seek the immediate closest instance node.
     * 3. Node not exists(node == NULL).
     */
    p_nbl = nbl_pop(&stk_top, &stk_buttom);
    if (p_nbl == NULL) {
      /* End of traversal. */
      oid_cpy(ret_oid->oid, root_oid, root_oid_len);
      ret_oid->id_len = root_oid_len;
      ret_oid->inst_id = NULL;
      ret_oid->inst_id_len = 0;
      ret_oid->exist_state = BER_TAG_END_OF_MIB_VIEW;
      return (struct mib_node *)&mib_dummy_node;
    }
    oid--;  /* OID length is ignored once backtracking. */
    node = p_nbl->node;
    immediate = 1;  /* Switch to the immediate search mode. */
  }

  assert(0);
  return node;
}

/* Check if mib root node is initialized */
static inline void
mib_tree_init_check(void)
{
  if (mib_dummy_node.sub_id == NULL || mib_dummy_node.sub_ptr == NULL)
    die("Mib tree not init yet!");
}

/* Test a newly allocated node. */
static inline int
is_raw_group(struct mib_group_node *gn)
{
  assert(gn->sub_id != NULL);
  return gn->sub_id[0] == 0 && gn->sub_id_cnt == 0;
}

/* Resize group node's sub-id array */
#define alloc_nr(x) (((x)+5)*3/2)
static void
group_node_expand(struct mib_group_node *gn, int index)
{
  int i;

  assert(!is_raw_group(gn));

  if (gn->sub_id_cnt + 1 > gn->sub_id_cap) {
    /* resize new sub-id array */
    oid_t *sub_id = xcalloc(alloc_nr(gn->sub_id_cap), sizeof(oid_t));
    void **sub_ptr = xcalloc(alloc_nr(gn->sub_id_cap), sizeof(void *));

    gn->sub_id_cap = alloc_nr(gn->sub_id_cap);

    /* duplicate and insert */
    for (i = 0; i < gn->sub_id_cnt; i++) {
      if (i < index) {
        sub_id[i] = gn->sub_id[i];
        sub_ptr[i] = gn->sub_ptr[i];
      } else {
        sub_id[i + 1] = gn->sub_id[i];
        sub_ptr[i + 1] = gn->sub_ptr[i];
      }
    }

    /* abandon old ones */
    free(gn->sub_id);
    free(gn->sub_ptr);
    gn->sub_id = sub_id;
    gn->sub_ptr = sub_ptr;
  } else {
    /* Insert new sub-node(s) without expanding */
    for (i = gn->sub_id_cnt - 1; i >= index; i--) {
      gn->sub_id[i + 1] = gn->sub_id[i];
      gn->sub_ptr[i + 1] = gn->sub_ptr[i];
    }
  }
}

/* Shrink group node's sub-id array when removing sub-node(s) */
static void
group_node_shrink(struct mib_group_node *gn, int index)
{
  int i;

  if (gn->sub_id_cnt > 1) {
    for (i = index; i < gn->sub_id_cnt - 1; i++) {
      gn->sub_id[i] = gn->sub_id[i + 1];
      gn->sub_ptr[i] = gn->sub_ptr[i + 1];
    }
    gn->sub_id_cnt--;
  } else {
    gn->sub_id[0] = 0;
    gn->sub_ptr[0] = NULL;
    gn->sub_id_cnt = 0;
  }
}

static struct mib_group_node *
mib_group_node_new(void)
{
  struct mib_group_node *gn = xmalloc(sizeof(*gn));
  gn->type = MIB_OBJ_GROUP;
  gn->sub_id_cap = 1;
  gn->sub_id_cnt = 0;
  gn->sub_id = xcalloc(1, sizeof(oid_t));
  gn->sub_ptr = xcalloc(1, sizeof(void *));
  return gn;
}

static void
mib_group_node_delete(struct mib_group_node *gn)
{
  if (gn != NULL) {
    free(gn->sub_id);
    free(gn->sub_ptr);
    free(gn);
  }
}

static struct mib_instance_node *
mib_instance_node_new(int callback)
{
  struct mib_instance_node *in = xmalloc(sizeof(*in));
  in->type = MIB_OBJ_INSTANCE;
  in->callback = callback;
  return in;
}

static void
mib_instance_node_delete(struct mib_instance_node *in)
{
  if (in != NULL) {
    mib_handler_unref(in->callback);
    free(in);
  }
}

/* parent-child relationship */
struct node_pair {
  struct mib_node *parent;
  struct mib_node *child;
  int sub_idx;
};

/* Find node as well as its parent according to given oid. */
static struct mib_node *
mib_tree_node_search(const oid_t *oid, uint32_t id_len, struct node_pair *pair)
{
  struct mib_group_node *gn;
  struct mib_node *parent = pair->parent = (struct mib_node *)&mib_dummy_node;
  struct mib_node *node = pair->child = parent;
  int sub_idx = 0;

  if (id_len < root_oid_len)
    return NULL;

  while (node != NULL && id_len > 0) {
    switch (node->type) {

      case MIB_OBJ_GROUP:
        gn = (struct mib_group_node *)node;
        int i = oid_binary_search(gn->sub_id, gn->sub_id_cnt, *oid);
        if (i >= 0) {
          /* Sub-id found, go on loop */
          oid++;
          id_len--;
          sub_idx = i;
          parent = node;
          node = gn->sub_ptr[i];
          continue;
        } else {
          /* Sub-id not found */
          pair->parent = parent;
          pair->child = node;
          pair->sub_idx = sub_idx;
          return NULL;
        }

      case MIB_OBJ_INSTANCE:
        pair->parent = parent;
        pair->child = node;
        pair->sub_idx = sub_idx;
        if (id_len != 1) {
          return NULL;
        }
        return node;

      default:
        assert(0);
        break;
    }
  }

  /* node == NULL || id_len == 0 */
  /* Note: If target oid is the dummy root node, then
   * pair->parent == pair->child and pair->sub_idx == 0 */
  pair->parent = parent;
  pair->child = node;
  pair->sub_idx = sub_idx;
  return node;
}

/* Remove sub-node(s) in mib-tree. */
static void
__mib_tree_delete(struct node_pair *pair)
{
  struct node_backlog nbl, *p_nbl;
  struct node_backlog nbl_stk[MIB_OID_MAX_LEN];
  struct node_backlog *stk_top, *stk_buttom;

  struct mib_node *node = pair->child;
  struct mib_group_node *gn;
  struct mib_instance_node *in;

  if (node == (struct mib_node *)&mib_dummy_node) {
    CREDO_SNMP_LOG(SNMP_LOG_WARNING, "MIB dummy root node cannot be deleted!\n");
    return;
  }

  /* Init something */
  p_nbl = NULL;
  stk_top = stk_buttom = nbl_stk;

  for (; ;) {

    if (node != NULL)
    switch (node->type) {

      case MIB_OBJ_GROUP:
        gn = (struct mib_group_node *)node;

        int i;
        if (p_nbl != NULL) {
          /* Fetch the sub-id next to the pop-up backlogged one. */
          i = p_nbl->n_idx;
          p_nbl = NULL;
        } else {
          /* Fetch the first sub-id. */
          i = 0;
        }

        if (i == -1) {
          /* Sub-tree is empty, delete this node and go on backtracking */
          mib_group_node_delete(gn);
          break;
        }

        if (i + 1 >= gn->sub_id_cnt) {
          /* Last sub-id, mark n_idx = -1. */
          nbl.n_idx = -1;
        } else {
          nbl.n_idx = i + 1;
        }
        nbl.node = node;

        /* Backlog the current node and move down. */
        nbl_push(&nbl, &stk_top, &stk_buttom);
        node = gn->sub_ptr[i++];
        continue;

      case MIB_OBJ_INSTANCE:
        in = (struct mib_instance_node *)node;
        mib_instance_node_delete(in);
        break;

      default :
        assert(0);
        break;
    }

    /* Backtracking */
    p_nbl = nbl_pop(&stk_top, &stk_buttom);
    if (p_nbl == NULL) {
      /* End of traversal. */
      group_node_shrink((struct mib_group_node *)pair->parent, pair->sub_idx);
      return;
    }
    node = p_nbl->node;
  }
}

static void
mib_tree_delete(const oid_t *oid, uint32_t id_len)
{
  struct node_pair pair;
  struct mib_node *node;

  node = mib_tree_node_search(oid, id_len, &pair);
  if (node != NULL) {
    __mib_tree_delete(&pair);
  }
}

/* This function will create an instance node in mib-tree according to oid given
 * in which the prefix can be already created or not existing group node(s), and
 * the last id number must be the not existing instance node.
 */
static struct mib_instance_node *
mib_tree_instance_insert(const oid_t *oid, uint32_t id_len, int callback)
{
  struct mib_node *node = (struct mib_node *)&mib_dummy_node;
  struct mib_group_node *gn;

  if (id_len < root_oid_len)
    return NULL;

  while (id_len > 0) {
    switch (node->type) {

      case MIB_OBJ_GROUP:
        gn = (struct mib_group_node *)node;

        if (is_raw_group(gn)) {
          gn->sub_id_cnt++;
          gn->sub_id[0] = *oid++;
          if (--id_len == 0) {
            /* Allocate new instance node */
            node = gn->sub_ptr[0] = mib_instance_node_new(callback);
            return (struct mib_instance_node *)node;
          } else {
            /* Allocate new group node */
            node = gn->sub_ptr[0] = mib_group_node_new();
          }
        } else {
          /* Search in exist sub-ids */
          int i = oid_binary_search(gn->sub_id, gn->sub_id_cnt, *oid);
          if (i >= 0) {
            /* Sub-id found, go on traversing */
            oid++;
            id_len--;
            node = gn->sub_ptr[i];
          } else {
            /* Sub-id not found, that's it. */
            i = -i - 1;
            /* resize sub_id[] */
            group_node_expand(gn, i);
            gn->sub_id_cnt++;
            gn->sub_id[i] = *oid++;
            if (--id_len == 0) {
              /* Allocate new instance node */
              node = gn->sub_ptr[i] = mib_instance_node_new(callback);
              return (struct mib_instance_node *)node;
            } else {
              /* Allocate new group node */
              node = gn->sub_ptr[i] = mib_group_node_new();
            }
          }
        }
        continue;

      case MIB_OBJ_INSTANCE:
        /* Bad traversal */
        return NULL;

      default:
        assert(0);
        break;
    }
  }

  /* Bad traversal */
  return NULL;
}

/* Register one instance node in mib-tree according to given oid with lua callback. */
int
mib_node_reg(const oid_t *oid, uint32_t len, int callback)
{
  int i;
  struct mib_instance_node *in;

  mib_tree_init_check();

  in = mib_tree_instance_insert(oid, len, callback);
  if (in == NULL) {
    CREDO_SNMP_LOG(SNMP_LOG_WARNING, "Register oid \"");
    for (i = 0; i < len; i++) {
      CREDO_SNMP_LOG(SNMP_LOG_WARNING, ".%d", oid[i]);
    }
    CREDO_SNMP_LOG(SNMP_LOG_WARNING, "\" fail, node already exists or oid prefix not match root oid (");
    for (i = 0; i < root_oid_len; i++) {
      CREDO_SNMP_LOG(SNMP_LOG_WARNING, ".%d", root_oid[i]);
    }
    CREDO_SNMP_LOG(SNMP_LOG_WARNING, ")\n");
    return -1;
  }

  return 0;
}

/* Unregister node(s) in mib-tree according to given oid. */
void
mib_node_unreg(const oid_t *oid, uint32_t len)
{
  mib_tree_init_check();

  mib_tree_delete(oid, len);
}

/* Init dummy root node */
static void
mib_dummy_node_init(void)
{
  const oid_t dummy_oid[] = { 1, 3, 6, 1 };

  root_oid = xmalloc(OID_ARRAY_SIZE(dummy_oid) * sizeof(oid_t));
  oid_cpy(root_oid, dummy_oid, OID_ARRAY_SIZE(dummy_oid));
  root_oid_len = OID_ARRAY_SIZE(dummy_oid);

  mib_dummy_node.type = MIB_OBJ_GROUP;
  mib_dummy_node.sub_id_cap = 1;
  mib_dummy_node.sub_id_cnt = 0;
  mib_dummy_node.sub_id = xmalloc(sizeof(oid_t));
  mib_dummy_node.sub_id[0] = 0;
  mib_dummy_node.sub_ptr = xmalloc(sizeof(void *));
  mib_dummy_node.sub_ptr[0] = NULL;
}

void
mib_init(void)
{
  mib_dummy_node_init();
}

