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
#include "util.h"

/* MIB lua state */
lua_State *mib_lua_state;

/* Dummy root node */
static struct mib_group_node mib_dummy_node = {
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
  int ret = 0;

  while (src_len && tar_len && !(ret = (int)(*src++ - *target++))) {
    src_len--;
    tar_len--;
    continue;
  }

  if (!ret)
    return src_len - tar_len;
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

int
oid_cover(const oid_t *oid1, uint32_t len1, const oid_t *oid2, uint32_t len2)
{
  if (len1 <= len2 && !oid_cmp(oid1, len1, oid2, len1)) {
    /* oid1 covers oid2 */
    return 1;
  } else if (len2 <= len1 && !oid_cmp(oid1, len2, oid2, len2)) {
    /* oid2 covers oid1 */
    return -1;
  } else {
    return 0;
  }
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

/* Unrefer mib search handler */
void
mib_handler_unref(int handler)
{
  lua_State *L = mib_lua_state;
  luaL_unref(L, LUA_ENVIRONINDEX, handler);
}

/* Embedded code is not funny at all... */
int
mib_instance_search(struct oid_search_res *ret_oid)
{
  int i;
  Variable *var = &ret_oid->var;
  lua_State *L = mib_lua_state;

  /* Empty lua stack. */
  lua_pop(L, -1);
  /* Get function. */
  lua_rawgeti(L, LUA_ENVIRONINDEX, ret_oid->callback);
  /* op */
  lua_pushinteger(L, ret_oid->request);
  /* req_sub_oid */
  lua_newtable(L);
  for (i = 0; i < ret_oid->inst_id_len; i++) {
    lua_pushinteger(L, ret_oid->inst_id[i]);
    lua_rawseti(L, -2, i + 1);
  }

  if (ret_oid->request == MIB_REQ_SET) {
    /* req_val */
    switch (tag(var)) {
      case ASN1_TAG_INT:
        lua_pushinteger(L, integer(var));
        break;
      case ASN1_TAG_OCTSTR:
        lua_pushlstring(L, octstr(var), length(var));
        break;
      case ASN1_TAG_CNT:
        lua_pushnumber(L, count(var));
        break;
      case ASN1_TAG_IPADDR:
        lua_pushlstring(L, (char *)ipaddr(var), length(var));
        break;
      case ASN1_TAG_OBJID:
        lua_newtable(L);
        for (i = 0; i < length(var); i++) {
          lua_pushnumber(L, oid(var)[i]);
          lua_rawseti(L, -2, i + 1);
        }
        break;
      case ASN1_TAG_GAU:
        lua_pushnumber(L, gauge(var));
        break;
      case ASN1_TAG_TIMETICKS:
        lua_pushnumber(L, timeticks(var));
        break;
      default:
        lua_pushnil(L);
        break;
    }
    /* req_val_type */
    lua_pushinteger(L, tag(var));
  } else {
    /* req_val */
    lua_pushnil(L);
    /* req_val_type */
    lua_pushnil(L);
  }

  if (lua_pcall(L, 4, 4, 0) != 0) {
    SMARTSNMP_LOG(L_ERROR, "MIB search hander %d fail: %s\n", ret_oid->callback, lua_tostring(L, -1));
    tag(var) = ASN1_TAG_NO_SUCH_OBJ;
    return 0;
  }

  ret_oid->err_stat = lua_tointeger(L, -4);
  tag(var) = lua_tonumber(L, -1);

  if (!ret_oid->err_stat && MIB_TAG_VALID(tag(var))) {
    /* Return value */
    if (ret_oid->request != MIB_REQ_SET) {
      switch (tag(var)) {
        case ASN1_TAG_INT:
          length(var) = 1;
          integer(var) = lua_tointeger(L, -2);
          break;
        case ASN1_TAG_OCTSTR:
          length(var) = lua_objlen(L, -2);
          memcpy(octstr(var), lua_tostring(L, -2), length(var));
          break;
        case ASN1_TAG_CNT:
          length(var) = 1;
          count(var) = lua_tonumber(L, -2);
          break;
        case ASN1_TAG_IPADDR:
          length(var) = lua_objlen(L, -2);
          for (i = 0; i < length(var); i++) {
            lua_rawgeti(L, -2, i + 1);
            ipaddr(var)[i] = lua_tointeger(L, -1);
            lua_pop(L, 1);
          }
          break;
        case ASN1_TAG_OBJID:
          length(var) = lua_objlen(L, -2);
          for (i = 0; i < length(var); i++) {
            lua_rawgeti(L, -2, i + 1);
            oid(var)[i] = lua_tointeger(L, -1);
            lua_pop(L, 1);
          }
          break;
        case ASN1_TAG_GAU:
          length(var) = 1;
          gauge(var) = lua_tonumber(L, -2);
          break;
        case ASN1_TAG_TIMETICKS:
          length(var) = 1;
          timeticks(var) = lua_tonumber(L, -2);
          break;
        default:
          assert(0);
      }
    }

    /* For GETNEXT request, return the new oid */
    if (ret_oid->request == MIB_REQ_GETNEXT) {
      ret_oid->inst_id_len = lua_objlen(L, -3);
      for (i = 0; i < ret_oid->inst_id_len; i++) {
        lua_rawgeti(L, -3, i + 1);
        ret_oid->inst_id[i] = lua_tointeger(L, -1);
        lua_pop(L, 1);
      }
    }
  }

  return ret_oid->err_stat;
}

/* GET request search, depth-first traversal in mib-tree, oid must match */
struct mib_node *
mib_tree_search(struct mib_view *view, const oid_t *orig_oid, uint32_t orig_id_len, struct oid_search_res *ret_oid)
{
  oid_t *oid;
  uint32_t id_len;
  struct mib_node *node;
  struct mib_group_node *gn;
  struct mib_instance_node *in;

  assert(view != NULL && orig_oid != NULL && ret_oid != NULL);

  /* Duplicate OID as return value */
  ret_oid->oid = oid_dup(orig_oid, orig_id_len);
  ret_oid->id_len = orig_id_len;
  ret_oid->err_stat = 0;

  /* Access control */
  if (oid_cover(view->oid, view->id_len, orig_oid, orig_id_len) <= 0) {
    /* Out of range of view */
    tag(&ret_oid->var) = ASN1_TAG_NO_SUCH_OBJ;
    return NULL;
  }

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
          tag(&ret_oid->var) = ASN1_TAG_NO_SUCH_OBJ;
          return node;
        }

      case MIB_OBJ_INSTANCE:
        in = (struct mib_instance_node *)node;
        /* Find instance variable through lua handler function */
        ret_oid->inst_id = oid;
        ret_oid->inst_id_len = id_len;
        ret_oid->callback = in->callback;
        ret_oid->err_stat = mib_instance_search(ret_oid);
        return node;

      default:
        assert(0);
    }
  }

  /* node == NULL || id_len == 0 */
  ret_oid->inst_id = oid;
  ret_oid->inst_id_len = id_len;
  if (node && node->type == MIB_OBJ_INSTANCE) {
    tag(&ret_oid->var) = ASN1_TAG_NO_SUCH_INST;
  } else {
    tag(&ret_oid->var) = ASN1_TAG_NO_SUCH_OBJ;
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
void
mib_tree_search_next(struct mib_view *view, const oid_t *orig_oid, uint32_t orig_id_len, struct oid_search_res *ret_oid)
{
  oid_t *oid;
  uint32_t id_len;
  struct node_backlog nbl, *p_nbl;
  struct node_backlog nbl_stk[MIB_OID_MAX_LEN];
  struct node_backlog *stk_top, *stk_buttom;
  struct mib_node *node;
  struct mib_group_node *gn;
  struct mib_instance_node *in;
  /* 'immediate' is the search state indicator.
   * 0 is to get the matched instance according to the given oid;
   * 1 is to get the immediate first instance regardless of the given oid. */
  uint8_t immediate = 0;

  assert(view != NULL && orig_oid != NULL && ret_oid != NULL);

  /* Access control */
  if (oid_cover(view->oid, view->id_len, orig_oid, orig_id_len) > 0) {
    /* In the range of view, search the root node at view oid */
    ret_oid->request = MIB_REQ_GET;
    node = mib_tree_search(view, view->oid, view->id_len, ret_oid);
    assert(node != NULL);
    ret_oid->request = MIB_REQ_GETNEXT;
    /* Duplicate the given oid */
    oid_cpy(ret_oid->oid, orig_oid, orig_id_len);
    ret_oid->id_len = orig_id_len;
    if (ret_oid->id_len > ret_oid->inst_id - ret_oid->oid) {
      /* Given oid is longer than the search result's, we need to search according to the given oid */
      immediate = 0;
    } else {
      /* Otherwise, ignore the given oid */
      immediate = 1;
    }
  } else {
    /* Out of range of view */
    if (oid_cmp(orig_oid, orig_id_len, view->oid, view->id_len) < 0) {
      /* Given oid is ahead of view, search the root node at view oid */
      ret_oid->request = MIB_REQ_GET;
      node = mib_tree_search(view, view->oid, view->id_len, ret_oid);
      assert(node != NULL);
      ret_oid->request = MIB_REQ_GETNEXT;
      /* Set the search mode according to node type */
      if (node->type == MIB_OBJ_GROUP) {
        immediate = 1;
      } else {
        immediate = 0;
      }
    } else {
      /* END_OF_MIB_VIEW */
      node = NULL;
      ret_oid->oid = oid_dup(view->oid, view->id_len);
      ret_oid->id_len = view->id_len;
    }
  }

  /* Init something */
  p_nbl = NULL;
  stk_top = stk_buttom = nbl_stk;
  ret_oid->err_stat = 0;
  oid = ret_oid->inst_id;
  id_len = ret_oid->id_len - (oid - ret_oid->oid);

  for (; ;) {

    if (node != NULL) {
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
          ret_oid->err_stat = mib_instance_search(ret_oid);
          if (MIB_TAG_VALID(tag(&ret_oid->var))) {
            ret_oid->id_len = oid - ret_oid->oid + ret_oid->inst_id_len;
            assert(ret_oid->id_len <= MIB_OID_MAX_LEN);
            if (!oid_cover(view->oid, view->id_len, ret_oid->oid, ret_oid->id_len)) {
              /* End of mib view */
              break;
            }
            return;
          } else {
            /* Instance not found */
            break;
          }

        default:
          assert(0);
      }
    }

    /* Backtracking condition:
     * 1. No greater sub-id in group node;
     * 2. Seek the immediate closest instance node;
     * 3. Node not exists(node == NULL).
     */
    p_nbl = nbl_pop(&stk_top, &stk_buttom);
    if (p_nbl == NULL) {
      /* End of traversal. */
      oid_cpy(ret_oid->oid, orig_oid, orig_id_len);
      ret_oid->id_len = orig_id_len;
      ret_oid->inst_id = NULL;
      ret_oid->inst_id_len = 0;
      ret_oid->err_stat = 0;
      tag(&ret_oid->var) = ASN1_TAG_END_OF_MIB_VIEW;
      return;
    }
    oid--;  /* OID length is ignored once backtracking. */
    node = p_nbl->node;
    immediate = 1;  /* Switch to the immediate search mode. */
  }
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
    SMARTSNMP_LOG(L_WARNING, "MIB dummy root node cannot be deleted!\n");
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

  assert(oid != NULL);

  mib_tree_init_check();

  /* Prefix must match root oid */
  if (len == 0) {
    SMARTSNMP_LOG(L_WARNING, "The register group node oid cannot be empty\n");
    return -1;
  }

  if (len > MIB_OID_MAX_LEN) {
    SMARTSNMP_LOG(L_WARNING, "The register group oid cannot be longer than %d\n", MIB_OID_MAX_LEN);
    return -1;
  }

  in = mib_tree_instance_insert(oid, len, callback);
  if (in == NULL) {
    SMARTSNMP_LOG(L_WARNING, "Register group node oid: ");
    for (i = 0; i < len; i++) {
      SMARTSNMP_LOG(L_WARNING, "%d ", oid[i]);
    }
    SMARTSNMP_LOG(L_WARNING, "fail, node already exists or oid overlaps.\n");
    return -1;
  }

  return 0;
}

/* Unregister node(s) in mib-tree according to given oid. */
void
mib_node_unreg(const oid_t *oid, uint32_t len)
{
  assert(oid != NULL);
  mib_tree_init_check();
  mib_tree_delete(oid, len);
}

/* Init dummy root node */
static void
mib_dummy_node_init(void)
{
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
