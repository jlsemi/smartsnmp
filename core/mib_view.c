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

static struct mib_view *mib_views;
static struct mib_community *mib_communities;
static struct mib_user *mib_users;

static struct mib_view *
mib_view_search(const oid_t *oid, uint32_t id_len)
{
  struct mib_view *v;

  for (v = mib_views; v != NULL; v = v->next) {
    if (!oid_cmp(v->oid, v->id_len, oid, id_len)) {
      return v;
    }
  }

  return NULL;
}

static struct mib_view *
view_create(const oid_t *oid, uint32_t id_len)
{
  struct mib_view *v;

  v = mib_view_search(oid, id_len);
  if (v == NULL) {
    v = xmalloc(sizeof(*v));
    v->oid = oid_dup(oid, id_len);
    v->id_len = id_len;
    INIT_LIST_HEAD(&v->communities);
    INIT_LIST_HEAD(&v->users);
    v->next = mib_views;
    mib_views = v;
  }

  return v;
}

static struct mib_community *
community_create(const char *community)
{
  struct mib_community *c;

  c = mib_community_search(community);
  if (c == NULL) {
    c = xmalloc(sizeof(*c));
    char *name = xmalloc(strlen(community) + 1);
    c->name = strcpy(name, community);
    INIT_LIST_HEAD(&c->ro_views);
    INIT_LIST_HEAD(&c->rw_views);
    c->next = mib_communities;
    mib_communities = c;
  }

  return c;
}

static int
community_view_filter(const oid_t *oid, uint32_t id_len, struct list_head *views)
{
  struct list_head *curr, *next;

  list_for_each_safe(curr, next, views) {
    struct community_view *cv = list_entry(curr, struct community_view, clink);
    int cover = oid_cover(cv->view->oid, cv->view->id_len, oid, id_len);
    if (cover > 0) {
      return 1;
    } else if (cover < 0) {
      list_del(&cv->clink);
      list_del(&cv->vlink);
      free(cv);
    }
  }

  return 0;
}

static void
community_view_add(const oid_t *oid, uint32_t id_len,
                   struct mib_community *c, struct mib_view *v, struct list_head *views)
{
  struct community_view *cv;
  struct list_head *curr;

  cv = xmalloc(sizeof(*cv));
  cv->view = v;
  cv->community = c;
  INIT_LIST_HEAD(&cv->vlink);
  INIT_LIST_HEAD(&cv->clink);
  list_add(&cv->vlink, &v->communities);
  list_for_each(curr, views) {
    struct community_view *cv1 = list_entry(curr, struct community_view, clink);
    int cmp = oid_cmp(cv1->view->oid, cv1->view->id_len, oid, id_len);
    if (cmp > 0) {
      list_add_tail(&cv->clink, curr);
      return;
    }
  }
  list_add_tail(&cv->clink, views);
}

static void
community_view_remove(struct list_head *views)
{
  struct list_head *curr, *next;

  list_for_each_safe(curr, next, views) {
    struct community_view *cv = list_entry(curr, struct community_view, clink);
    list_del(&cv->vlink);
    list_del(&cv->clink);
    free(cv);
  }
}

static void
community_view_bind(const oid_t *oid, uint32_t id_len, struct mib_community *c, MIB_ACES_ATTR_E attribute)
{
  int filtered;
  struct mib_view *v;
  struct list_head *views;

  /* Access attribute */
  if (attribute == MIB_ACES_READ) {
    views = &c->ro_views;
  } else {
    views = &c->rw_views;
  }

  /* Filter out covered mib views in community list */
  filtered = community_view_filter(oid, id_len, views);
  if (!filtered) {
    /* Create new community and insert into mib view list */
    v = view_create(oid, id_len);
    /* Add new community view */
    community_view_add(oid, id_len, c, v, views);
  }
}

void
mib_community_reg(const oid_t *oid, uint32_t id_len, const char *community, MIB_ACES_ATTR_E attribute)
{
  struct mib_community *c;

  assert(oid != NULL && community != NULL);

  /* Create new community and insert into mib community list */
  c = community_create(community);

  /* Bind community-view */
  if (attribute == MIB_ACES_WRITE) {
    community_view_bind(oid, id_len, c, MIB_ACES_WRITE);
  }
  community_view_bind(oid, id_len, c, MIB_ACES_READ);
}

void
mib_community_unreg(const char *community, MIB_ACES_ATTR_E attribute)
{
  struct mib_community **cc = &mib_communities;

  assert(community != NULL);

  while (*cc != NULL) {
    struct mib_community *c = *cc;
    if (!strcmp(c->name, community)) {
      /* Delete all community views */
      if (attribute == MIB_ACES_READ) {
        community_view_remove(&c->ro_views);
      }
      community_view_remove(&c->rw_views);

      /* If both RW views emtpy, delete this community string */
      if (list_empty(&c->ro_views) && list_empty(&c->rw_views)) {
        *cc = c->next;
        free(c);
      }
    } else {
      cc = &c->next;
    }
  }
}

struct mib_community *
mib_community_search(const char *community)
{
  struct mib_community *c;

  if (community != NULL) {
    for (c = mib_communities; c != NULL; c = c->next) {
      if (!strcmp(c->name, community)) {
        return c;
      }
    }
  }

  return NULL;
}

int
mib_community_view_cover(struct mib_community *c, MIB_ACES_ATTR_E attribute, const oid_t *oid, uint32_t id_len)
{
  struct list_head *pos, *views;

  if (c != NULL) {
    /* Access attribute */
    if (attribute == MIB_ACES_READ) {
      views = &c->ro_views;
    } else {
      views = &c->rw_views;
    }

    list_for_each(pos, views) {
      struct community_view *cv = list_entry(pos, struct community_view, clink);
      struct mib_view *v = cv->view;
      if (oid_cover(v->oid, v->id_len, oid, id_len) > 0) {
        return 1;
      }
    }
  }

  return 0;
}

struct mib_view*
mib_community_next_view(struct mib_community *c, MIB_ACES_ATTR_E attribute, struct mib_view *v)
{
  struct list_head *pos, *views;

  if (c != NULL) {
    /* Access attribute */
    if (attribute == MIB_ACES_READ) {
      views = &c->ro_views;
    } else {
      views = &c->rw_views;
    }

    list_for_each(pos, views) {
      struct community_view *cv = list_entry(pos, struct community_view, clink);
      if (v == NULL) {
        /* Return first view */
        pos = views->next;
        cv = list_entry(pos, struct community_view, clink);
        return cv->view;
      } else if (v == cv->view) {
        if (!list_is_last(&cv->clink, views)) {
          /* Return next view */
          pos = cv->clink.next;
          cv = list_entry(pos, struct community_view, clink);
          return cv->view;
        }
      }
    }
  }

  return NULL;
}

static struct mib_user *
user_create(const char *user)
{
  struct mib_user *u;

  u = mib_user_search(user);
  if (u == NULL) {
    u = xmalloc(sizeof(*u));
    char *name = xmalloc(strlen(user) + 1);
    u->name = strcpy(name, user);
    INIT_LIST_HEAD(&u->ro_views);
    INIT_LIST_HEAD(&u->rw_views);
    u->next = mib_users;
    mib_users = u;
  }

  return u;
}

static int
user_view_filter(const oid_t *oid, uint32_t id_len, struct list_head *views)
{
  struct list_head *curr, *next;

  list_for_each_safe(curr, next, views) {
    struct user_view *uv = list_entry(curr, struct user_view, ulink);
    int cover = oid_cover(uv->view->oid, uv->view->id_len, oid, id_len);
    if (cover > 0) {
      return 1;
    } else if (cover < 0) {
      list_del(&uv->ulink);
      list_del(&uv->vlink);
      free(uv);
    }
  }

  return 0;
}

static void
user_view_add(const oid_t *oid, uint32_t id_len,
              struct mib_user *u, struct mib_view *v, struct list_head *views)
{
  struct user_view *uv;
  struct list_head *curr;

  uv = xmalloc(sizeof(*uv));
  uv->view = v;
  uv->user = u;
  INIT_LIST_HEAD(&uv->vlink);
  INIT_LIST_HEAD(&uv->ulink);
  list_add(&uv->vlink, &v->users);
  list_for_each(curr, views) {
    struct user_view *uv1 = list_entry(curr, struct user_view, ulink);
    int cmp = oid_cmp(uv1->view->oid, uv1->view->id_len, oid, id_len);
    if (cmp > 0) {
      list_add_tail(&uv->ulink, curr);
      return;
    }
  }
  list_add_tail(&uv->ulink, views);
}

static void
user_view_remove(struct list_head *views)
{
  struct list_head *curr, *next;

  list_for_each_safe(curr, next, views) {
    struct user_view *uv = list_entry(curr, struct user_view, ulink);
    list_del(&uv->vlink);
    list_del(&uv->ulink);
    free(uv);
  }
}

static void
user_view_bind(const oid_t *oid, uint32_t id_len, struct mib_user *u, MIB_ACES_ATTR_E attribute)
{
  int filtered;
  struct mib_view *v;
  struct list_head *views;

  /* Access attribute */
  if (attribute == MIB_ACES_READ) {
    views = &u->ro_views;
  } else {
    views = &u->rw_views;
  }

  /* Filter out covered mib views in user list */
  filtered = user_view_filter(oid, id_len, views);
  if (!filtered) {
    /* Create new user and insert into mib view list */
    v = view_create(oid, id_len);
    /* Add new user view */
    user_view_add(oid, id_len, u, v, views);
  }
}

void
mib_user_reg(const oid_t *oid, uint32_t id_len, const char *user, MIB_ACES_ATTR_E attribute)
{
  struct mib_user *u;

  assert(oid != NULL && user != NULL);

  /* Create new user and insert into mib user list */
  u = user_create(user);

  /* Bind user-view */
  if (attribute == MIB_ACES_WRITE) {
    user_view_bind(oid, id_len, u, MIB_ACES_WRITE);
  }
  user_view_bind(oid, id_len, u, MIB_ACES_READ);
}

void
mib_user_unreg(const char *user, MIB_ACES_ATTR_E attribute)
{
  struct mib_user **uu = &mib_users;

  assert(user != NULL);

  while (*uu != NULL) {
    struct mib_user *u = *uu;
    if (!strcmp(u->name, user)) {
      /* Delete all user views */
      if (attribute == MIB_ACES_READ) {
        user_view_remove(&u->ro_views);
      }
      user_view_remove(&u->rw_views);

      /* If both RW views emtpy, delete this user */
      if (list_empty(&u->ro_views) && list_empty(&u->rw_views)) {
        *uu = u->next;
        free(u);
      }
    } else {
      uu = &u->next;
    }
  }
}

struct mib_user *
mib_user_search(const char *user)
{
  struct mib_user *u;

  if (user != NULL) {
    for (u = mib_users; u != NULL; u = u->next) {
      if (!strcmp(u->name, user)) {
        return u;
      }
    }
  }

  return NULL;
}

int
mib_user_view_cover(struct mib_user *u, MIB_ACES_ATTR_E attribute, const oid_t *oid, uint32_t id_len)
{
  struct list_head *pos, *views;

  if (u != NULL) {
    /* Access attribute */
    if (attribute == MIB_ACES_READ) {
      views = &u->ro_views;
    } else {
      views = &u->rw_views;
    }

    list_for_each(pos, views) {
      struct user_view *uv = list_entry(pos, struct user_view, ulink);
      struct mib_view *v = uv->view;
      if (oid_cover(v->oid, v->id_len, oid, id_len) > 0) {
        return 1;
      }
    }
  }

  return 0;
}

struct mib_view *
mib_user_next_view(struct mib_user *u, MIB_ACES_ATTR_E attribute, struct mib_view *v)
{
  struct list_head *pos, *views;

  if (u != NULL) {
    /* Access attribute */
    if (attribute == MIB_ACES_READ) {
      views = &u->ro_views;
    } else {
      views = &u->rw_views;
    }

    list_for_each(pos, views) {
      struct user_view *uv = list_entry(pos, struct user_view, ulink);
      if (v == NULL) {
        /* Return first view */
        pos = views->next;
        uv = list_entry(pos, struct user_view, ulink);
        return uv->view;
      } else if (v == uv->view) {
        if (!list_is_last(&uv->ulink, views)) {
          /* Return next view */
          pos = uv->ulink.next;
          uv = list_entry(pos, struct user_view, ulink);
          return uv->view;
        }
      }
    }
  }

  return NULL;
}
