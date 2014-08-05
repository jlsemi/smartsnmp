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

/* Internet group */
static struct mib_group_node internet_group = {
  MIB_OBJ_GROUP,
  1,
  0,
  NULL,
  NULL
};

oid_t *
oid_dup(const oid_t *oid, uint32_t len)
{
  int i;
  oid_t *new_oid;

  new_oid = xmalloc(MIB_OID_MAX_LEN * sizeof(oid_t));
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

  /* Duplicate OID as return */
  ret_oid->oid = oid_dup(orig_oid, orig_id_len);
  ret_oid->id_len = orig_id_len;
  ret_oid->exist_state = 0;

  /* Check dummy root oid prefix */
  if (orig_id_len < INTERNET_PREFIX_LENGTH) {
    node = NULL;
    oid = NULL;
    id_len = 0;
  } else {
    node = (struct mib_node *)&internet_group;
    oid = ret_oid->oid + INTERNET_PREFIX_LENGTH;
    id_len = ret_oid->id_len - INTERNET_PREFIX_LENGTH;
  }

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
        /* Find the instance */
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
  if (*top - *buttom < NBL_STACK_SIZE) {
    *((*top)++) = *nbl;
  }
}

static inline struct node_backlog *
nbl_pop(struct node_backlog **top, struct node_backlog **buttom)
{
  if (*top > *buttom)
    return --*top;

  return NULL;
}

/* GETNEXT request search, depth-first traversal in mib-tree, find the closest next oid. */
struct mib_node *
mib_tree_search_next(const oid_t *orig_oid, uint32_t orig_id_len, struct oid_search_res *ret_oid)
{
  oid_t *oid;
  const oid_t dummy_oid[] = { 1, 3, 6, 1 };
  uint32_t id_len;
  uint8_t immediate; /* This is the search state machine */

  struct node_backlog nbl, *p_nbl;
  struct node_backlog nbl_stk[NBL_STACK_SIZE];
  struct node_backlog *stk_top, *stk_buttom;

  struct mib_node *node;
  struct mib_group_node *gn;
  struct mib_instance_node *in;

  /* Init something first */
  stk_top = stk_buttom = nbl_stk;
  node = (struct mib_node *)&internet_group;
  p_nbl = NULL;

  /* Check dummy root oid prefix */
  if (orig_id_len > INTERNET_PREFIX_LENGTH) {
    immediate = 0;  /* 0 is to search the first match node in mib tree. */
    ret_oid->oid = oid_dup(orig_oid, orig_id_len);
    ret_oid->id_len = orig_id_len;
    if (oid_cmp(orig_oid, OID_ARRAY_SIZE(dummy_oid), dummy_oid, OID_ARRAY_SIZE(dummy_oid)) > 0) {
      /* END_OF_MIB_VIEW */
      oid = ret_oid->oid + orig_id_len;
      id_len = ret_oid->id_len - orig_id_len;
      node = NULL;
    } else {
      oid = ret_oid->oid + INTERNET_PREFIX_LENGTH;
      id_len = ret_oid->id_len - INTERNET_PREFIX_LENGTH;
    }
  } else {
    immediate = 1;  /* 1 is to get the immediate closest instance */
    if (oid_cmp(orig_oid, orig_id_len, dummy_oid, OID_ARRAY_SIZE(dummy_oid)) > 0) {
      /* END_OF_MIB_VIEW */
      ret_oid->oid = oid_dup(orig_oid, orig_id_len);
      ret_oid->id_len = orig_id_len;
      oid = ret_oid->oid + orig_id_len;
      id_len = ret_oid->id_len - orig_id_len;
      node = NULL;
    } else {
      ret_oid->oid = oid_dup(dummy_oid, OID_ARRAY_SIZE(dummy_oid));
      ret_oid->id_len = OID_ARRAY_SIZE(dummy_oid);
      oid = ret_oid->oid + INTERNET_PREFIX_LENGTH;
      id_len = ret_oid->id_len - INTERNET_PREFIX_LENGTH;
    }
  }
  ret_oid->inst_id = NULL;
  ret_oid->inst_id_len = 0;
  ret_oid->exist_state = 0;

  for (; ;) {

    if (node != NULL)
    switch (node->type) {

      case MIB_OBJ_GROUP:
        gn = (struct mib_group_node *)node;

        if (immediate) {
          /* Get the immediate closest instance. */
          int i;

          if (p_nbl != NULL) {
            /* That pop-up backlog recorded the next index. */
            i = p_nbl->n_idx;
            p_nbl = NULL;
          } else {
            /* else we fetch the immediate first sub-id. */
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
            /* Not found, oid matching is ignored since here */
            immediate = 1;

            /* Reverse the sign to locate the next sub-id. */
            i = -i - 1;
            if (i == gn->sub_id_cnt) {
              /* All sub-ids are greater than target;
               * Backtrack and get the next one. */
              break;
            } else if (i == 0) {
              /* 1. All sub-ids are less than target;
               * 2. No sub-id in this group node;
               * Just switch to immediate mode */
              continue;
            } /* else {
              Target is between the two sub-ids and [i] is the next one, switch to immediate mode and move on.
            } */
          }

          /* Sub-id found is greater or just equal to target,
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
            /* When oid length is decreased to zero, switch to the 'immediate' mode */
            immediate = 1;
          }
        }

        continue; /* Go on loop */

      case MIB_OBJ_INSTANCE:
        in = (struct mib_instance_node *)node;

        if (immediate || id_len == 0) {
          /* Get the first instance */
          ret_oid->inst_id_len = 0;
        } else {
          /* Search the closest instance whose oid greater than target */
          ret_oid->inst_id_len = id_len;
        }

        /* Find instance */
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
      ret_oid->id_len = oid - ret_oid->oid;
      ret_oid->inst_id = NULL;
      ret_oid->inst_id_len = 0;
      ret_oid->exist_state = BER_TAG_END_OF_MIB_VIEW;
      return (struct mib_node *)&internet_group;
    }
    oid--;  /* OID length is ignored once backtracking. */
    node = p_nbl->node;
    immediate = 1;  /* Switch to the 'immediate' mode. */
  }

  assert(0);
  return node;
}

/* Check if mib root node is initialized */
static inline void
mib_tree_init_check(void)
{
  if (internet_group.sub_id == NULL || internet_group.sub_ptr == NULL)
    die("Mib tree not init yet!");
}

/* Test a newly allocated group node. */
static inline int
is_raw_group_node(struct mib_group_node *gn)
{
  assert(gn->sub_id != NULL);
  return gn->sub_id[0] == 0 && gn->sub_id_cnt == 0;
}

/* Resize group node's sub-id array */
#define alloc_nr(x) (((x)+5)*3/2)
static void
group_node_resize(struct mib_group_node *gn, int index)
{
  int i;

  assert(!is_raw_group_node(gn));

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
    /* just insert */
    for (i = gn->sub_id_cnt - 1; i >= index; i--) {
      gn->sub_id[i + 1] = gn->sub_id[i];
      gn->sub_ptr[i + 1] = gn->sub_ptr[i];
    }
  }
}

/* Shrink group node's sub-id array when removing sub-node */
static void
group_node_shrink(struct mib_group_node *gn, int index)
{
  if (gn->sub_id_cnt > 1) {
    int i;
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
mib_instance_node_new(int lua_callback)
{
  struct mib_instance_node *in = xmalloc(sizeof(*in));
  in->type = MIB_OBJ_INSTANCE;
  in->callback = lua_callback;
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

/* Find node through oid and get its parent. */
static struct mib_node *
mib_tree_node_search(const oid_t *oid, uint32_t id_len, struct node_pair *pair)
{
  struct mib_group_node *gn;
  struct mib_node *parent = pair->parent = (struct mib_node *)&internet_group;
  struct mib_node *node = pair->child = parent;
  int sub_idx = 0;

  /* Internet group is the dummy root node */
  if (id_len < INTERNET_PREFIX_LENGTH)
    return NULL;

  oid += INTERNET_PREFIX_LENGTH;
  id_len -= INTERNET_PREFIX_LENGTH;

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
  /* Note: If target oid is the internet group node, then
   * pair->parent == pair->child and pair->sub_idx == 0 */
  pair->parent = parent;
  pair->child = node;
  pair->sub_idx = sub_idx;
  return node;
}

/* Remove specified node int mib-tree. */
static void
__mib_tree_delete(struct node_pair *pair)
{
  struct node_backlog nbl, *p_nbl;
  struct node_backlog nbl_stk[NBL_STACK_SIZE];
  struct node_backlog *stk_top, *stk_buttom;

  struct mib_node *node = pair->child;
  struct mib_group_node *gn;
  struct mib_instance_node *in;

  /* Internet group is the dummy root node */
  if (node == (struct mib_node *)&internet_group) {
    CREDO_SNMP_LOG(SNMP_LOG_WARNING, "OID .1.3.6.1 is the dummy root node in this mib tree which cannot be deleted.\n");
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
          /* That pop-up backlog recorded the next index. */
          i = p_nbl->n_idx;
          p_nbl = NULL;
        } else {
          /* Else we get the first sub-id. */
          i = 0;
        }

        if (i == -1) {
          /* All sub-trees empty, free this node and go on backtracking */
          mib_group_node_delete(gn);
          break;
        }

        do {
          if (i + 1 >= gn->sub_id_cnt) {
            /* Last sub-id, mark n_idx = -1. */
            nbl.n_idx = -1;
          } else {
            nbl.n_idx = i + 1;
          }
          nbl.node = node;
          node = gn->sub_ptr[i++];
        } while (node == NULL && i < gn->sub_id_cnt);

        /* Backlog the current node and move down. */
        nbl_push(&nbl, &stk_top, &stk_buttom);
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

/* This function create group node(s) in mib-tree according to oid given in
 * which the prefix can be already created as group node(s) and the last id
 * number must not be created yet. the id number before the last one will be
 * created as new dummy group node(s).
 */
static struct mib_group_node *
mib_tree_group_insert(const oid_t *oid, uint32_t id_len)
{
  struct mib_node *node;
  struct mib_group_node *gn;
  struct node_pair pair;

  /* Init something */
  pair.parent = pair.child = NULL;
  node = (struct mib_node *)&internet_group;

  oid += INTERNET_PREFIX_LENGTH;
  id_len -= INTERNET_PREFIX_LENGTH;

  while (id_len > 0) {
    switch (node->type) {

      case MIB_OBJ_GROUP:
        gn = (struct mib_group_node *)node;

        if (is_raw_group_node(gn)) {
          /* Allocate intermediate group node */
          node = gn->sub_ptr[0] = mib_group_node_new();
          gn->sub_id_cnt++;
          gn->sub_id[0] = *oid++;
          id_len--;
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
            /* realloc sub_id[] */
            group_node_resize(gn, i);
            /* Allocate new group node */
            node = gn->sub_ptr[i] = mib_group_node_new();
            gn->sub_id_cnt++;
            gn->sub_id[i] = *oid++;
            id_len--;
            /* Record the new allocated root group node */
            if (pair.child == NULL && pair.parent == NULL) {
              pair.parent = (struct mib_node *)gn;
              pair.child = (struct mib_node *)node;
              pair.sub_idx = i;
            }
          }
        }
        continue;

      case MIB_OBJ_INSTANCE:
        /* Bad traversal, clear all temporarily built group nodes */
        __mib_tree_delete(&pair);
        return NULL;

      default:
        assert(0);
        break;
    }
  }

  /* id_len == 0 */
  return (struct mib_group_node *)node;
}

/* This function creates one new instance node in mib-tree according to given
 * oid in which the last id number is the first id of the instance. Id number
 * before the last one are regarded as exist group node(s) by default.
 */
static struct mib_instance_node *
mib_tree_instance_insert(const oid_t *oid, uint32_t id_len, int lua_callback)
{
  struct mib_node *node;
  struct mib_group_node *gn;

  node = (struct mib_node *)&internet_group;
  oid += INTERNET_PREFIX_LENGTH;
  id_len -= INTERNET_PREFIX_LENGTH;

  while (node != NULL && id_len > 0) {
    switch (node->type) {

      case MIB_OBJ_GROUP:
        gn = (struct mib_group_node *)node;
        if (is_raw_group_node(gn) && id_len == 1) {
          /* Allocate intermediate group node */
          gn->sub_ptr[0] = mib_instance_node_new(lua_callback);
          gn->sub_id[0] = *oid;
          gn->sub_id_cnt++;
          return gn->sub_ptr[0];
        } else {
          /* Search in exist sub-ids */
          int i = oid_binary_search(gn->sub_id, gn->sub_id_cnt, *oid);
          if (i >= 0) {
            /* Sub-id found, go on traversing */
            oid++;
            id_len--;
            node = gn->sub_ptr[i];
            continue;
          } else {
            /* Sub-id not found, that's it. */
            i = -i - 1;
            if (id_len == 1) {
              /* The last only oid is for the new instance node */
              group_node_resize(gn, i);
              gn->sub_ptr[i] = mib_instance_node_new(lua_callback);
              gn->sub_id[i] = *oid;
              gn->sub_id_cnt++;
              return gn->sub_ptr[i];
            }
          }
        }

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

/* Register one instance node in mib-tree according to oid with callback defined
 * in Lua files. */
int
mib_node_reg(const oid_t *oid, uint32_t len, int lua_callback)
{
  int i;
  struct mib_group_node *gn;
  struct mib_instance_node *in;

  mib_tree_init_check();

  gn = mib_tree_group_insert(oid, len - 1);
  if (gn == NULL) {
    goto NODE_REG_FAIL;
  }

  in = mib_tree_instance_insert(oid, len, lua_callback);
  if (in == NULL) {
    goto NODE_REG_FAIL;
  }

  return 0;

NODE_REG_FAIL:
  CREDO_SNMP_LOG(SNMP_LOG_WARNING, "Register oid: ");
  for (i = 0; i < len; i++) {
    CREDO_SNMP_LOG(SNMP_LOG_WARNING, "%d ", oid[i]);
  }
  CREDO_SNMP_LOG(SNMP_LOG_WARNING, "fail, node already exists or oid overlaps.\n");
  return -1;
}

/* Unregister node(s) in mib-tree according to oid. */
void
mib_node_unreg(const oid_t *oid, uint32_t len)
{
  mib_tree_init_check();

  mib_tree_delete(oid, len);
}

static void
mib_internet_group_init(void)
{
  struct mib_group_node *gn = &internet_group;
  gn->type = MIB_OBJ_GROUP;
  gn->sub_id_cap = 1;
  gn->sub_id_cnt = 0;
  gn->sub_id = xmalloc(sizeof(oid_t));
  gn->sub_id[0] = 0;
  gn->sub_ptr = xmalloc(sizeof(void *));
  gn->sub_ptr[0] = NULL;
}

void
mib_init(void)
{
  mib_internet_group_init();
}

