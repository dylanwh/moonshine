#include <moonshine/config.h>
#include <moonshine/lua.h>
#include "avl/avl.h"

#define CLASS "moonshine.tree"

struct luatree/*{{{*/
{
	struct avl_tree_t *avltree;
	MSLuaRef *keycmp; /* use strcmp if null */
};/*}}}*/

struct luaitem/*{{{*/
{
	MSLuaRef *key, *value;
	struct luatree *tree;
};/*}}}*/

static void free_item(void *vp_item) {
	struct luaitem *item = vp_item;

	ms_lua_unref(item->key);
	ms_lua_unref(item->value);

	g_free(item);
}

static int compare_keys(const void *vp_b)
/*{{{*/
{
	const struct luaitem *b = vp_b;
	struct luatree *tree = b->tree;

	int result;

	if (tree->keycmp) {
		LuaState *L = ms_lua_pushref(tree->keycmp);
		lua_pushvalue(L, -2); /* re-push key A */
		LuaState *Lb = ms_lua_pushref(b->key);
		g_assert(L == Lb);

		lua_call(L, 2, 1);
		
		if (lua_type(L, -1) != LUA_TNUMBER)
			return luaL_error(L, "tree key compare: return value from comparator is not an integer");

		result = lua_tointeger(L, -1);
		lua_pop(L, 1);
	} else {
		size_t lenA, lenB;
		const char *strA, *strB;

		LuaState *L = ms_lua_pushref(b->key);

		/* builtin strcmp */
		strA = lua_tolstring(L, -2, &lenA);
		strB = lua_tolstring(L, -1, &lenB);

		result = memcmp(strA, strB, (lenA > lenB ? lenB : lenA) + 1);
		lua_pop(L, 1);
	}

	return result;
}/*}}}*/

/* On entry: Key in -1, value in -2 */
static avl_node_t *create_node(LuaState *L, struct luatree *tree)/*{{{*/
{
	struct luaitem *item = g_malloc(sizeof(*item));
	item->tree = tree;
	item->key = ms_lua_ref(L, -1);
	item->value = ms_lua_ref(L, -2);
	
	avl_node_t *node = avl_init_node(g_malloc(sizeof(*node)), item);

	return node;
}/*}}}*/

/* On entry: Key in -1, value in -2
 *
 * On exit: Old value (or nil) in -1. both entry values popped
 */

static void do_insert_replace(LuaState *L, struct luatree *tree)/*{{{*/
{
	avl_node_t *node;
	/* We can't just use the builtin avl tree insert, as it doesn't have a way
	 * to easily replace a node without doing a second search... and doesn't
	 * know about the lua stack.
	 *
	 * So, open-code it here.
	 */

	if (!tree->avltree->top) {
		avl_insert_top(tree->avltree, create_node(L, tree));
		goto ret_nil;
	}

	switch (avl_search_closest(tree->avltree, &node)) {
		case -1:
			avl_insert_before(tree->avltree, node, create_node(L, tree));
			goto ret_nil;
		case 1:
			avl_insert_after(tree->avltree, node, create_node(L, tree));
			goto ret_nil;
		case 0:
			{
				/* Replace the existing value */
				struct luaitem *item = node->item;
				MSLuaRef *old_value = item->value;
				item->value = ms_lua_ref(L, -2);
				
				/* Clean up, then push the old value onto the stack */
				lua_pop(L, 2);
				ms_lua_pushref(old_value);
				return;
			}
	}

ret_nil:
	lua_pop(L, 2);
	lua_pushnil(L);
}/*}}}*/


/* Lua interface */

static int tree_new(LuaState *L)/*{{{*/
{
	MSLuaRef *keycmp = NULL;

	if (lua_gettop(L) > 1)
		return luaL_argerror(L, 2, "Too many arguments");

	if (lua_gettop(L)) {
		luaL_checktype(L, 1, LUA_TFUNCTION);
		keycmp = ms_lua_ref(L, 1);
	}

	struct luatree *tree = ms_lua_newclass(L, CLASS, sizeof(*tree));
	tree->keycmp = keycmp;
	tree->avltree = avl_alloc_tree(compare_keys, free_item);
	
	return 1;
}/*}}}*/

static int tree_gc(LuaState *L)/*{{{*/
{
	struct luatree *tree = ms_lua_toclass(L, CLASS, 1);
	avl_free_tree(tree->avltree);
	if (tree->keycmp)
		ms_lua_unref(tree->keycmp);
	return 0;
}/*}}}*/

static int tree_tostring(LuaState *L)/*{{{*/
{
	char buff[32];
	sprintf(buff, "%p", ms_lua_toclass(L, CLASS, 1));
	lua_pushfstring(L, "Tree (%s)", buff);
	return 1;
}/*}}}*/

static int tree_insert(LuaState *L)/*{{{*/
{
	struct luatree *tree = ms_lua_toclass(L, CLASS, 1);
	luaL_argcheck(L, lua_gettop(L) == 3, lua_gettop(L), "Wrong number of arguments (need 3)");

	/* key at -1, value at -2 */
	lua_pushvalue(L, 3); /* value */
	lua_pushvalue(L, 2); /* key */

	do_insert_replace(L, tree);
	return 1;
}/*}}}*/

static int tree_delete(LuaState *L)/*{{{*/
{
	struct luatree *tree = ms_lua_toclass(L, CLASS, 1);
	luaL_argcheck(L, lua_gettop(L) == 2, lua_gettop(L), "Wrong number of arguments (need 2)");

	lua_pushvalue(L, 2); /* put key at -1 */
	avl_node_t *node = avl_search(tree->avltree);
	lua_pop(L, 1); /* remove key */

	if (!node) {
		lua_pushnil(L);
		return 1;
	}

	struct luaitem *item = node->item;
	ms_lua_pushref(item->value);
	avl_delete_node(tree->avltree, node);

	return 1;
}/*}}}*/

static int tree_clear(LuaState *L)/*{{{*/
{
	struct luatree *tree = ms_lua_toclass(L, CLASS, 1);
	luaL_argcheck(L, lua_gettop(L) == 1, lua_gettop(L), "Wrong number of arguments (need 1)");

	avl_free_nodes(tree->avltree);
	avl_init_tree(tree->avltree, compare_keys, free_item);

	return 0;
}/*}}}*/

static int tree_find(LuaState *L)/*{{{*/
{
	struct luatree *tree = ms_lua_toclass(L, CLASS, 1);
	luaL_argcheck(L, lua_gettop(L) == 2, lua_gettop(L), "Wrong number of arguments (need 2)");

	lua_pushvalue(L, 2); /* put key at -1 */
	avl_node_t *node = avl_search(tree->avltree);
	lua_pop(L, 1); /* remove key */

	if (!node) {
		lua_pushnil(L);
		lua_pushnil(L);
	} else {
		struct luaitem *item = node->item;
		ms_lua_pushref(item->value);
		lua_pushinteger(L, avl_index(node));
	}

	return 2;
}/*}}}*/

static int tree_find_near(LuaState *L)/*{{{*/
{
	struct luatree *tree = ms_lua_toclass(L, CLASS, 1);
	luaL_argcheck(L, lua_gettop(L) == 2, lua_gettop(L), "Wrong number of arguments (need 2)");

	lua_pushvalue(L, 2); /* put key at -1 */
	avl_node_t *node;
	int ret = avl_search_closest(tree->avltree, &node);
	lua_pop(L, 1); /* remove key */

	if (!node) {
		lua_pushnil(L);
		lua_pushnil(L);
		lua_pushnil(L);
	} else {
		struct luaitem *item = node->item;
		ms_lua_pushref(item->key);
		ms_lua_pushref(item->value);
		lua_pushinteger(L, avl_index(node));
	}
	lua_pushinteger(L, ret);

	return 4;
}/*}}}*/

static int tree_lookup_index(LuaState *L)/*{{{*/
{
	struct luatree *tree = ms_lua_toclass(L, CLASS, 1);
	luaL_argcheck(L, lua_gettop(L) == 2, lua_gettop(L), "Wrong number of arguments (need 2)");
	int v = luaL_checkinteger(L, 2);

	avl_node_t *node = avl_at(tree->avltree, v);
	if (!node) {
		lua_pushnil(L);
		lua_pushnil(L);
	} else {
		struct luaitem *item = node->item;
		ms_lua_pushref(item->key);
		ms_lua_pushref(item->value);
	}
	return 2;
}/*}}}*/

static int tree_size(LuaState *L)/*{{{*/
{
	struct luatree *tree = ms_lua_toclass(L, CLASS, 1);
	luaL_argcheck(L, lua_gettop(L) == 1, lua_gettop(L), "Wrong number of arguments (need 1)");

	lua_pushinteger(L, avl_count(tree->avltree));
	return 1;
}/*}}}*/

static const LuaLReg tree_methods[] = {/*{{{*/
	{"new", tree_new},
	{"insert", tree_insert},
	{"delete", tree_delete},
	{"clear", tree_clear},
	{"find",tree_find},
	{"find_near",tree_find_near},
	{"lookup_index",tree_lookup_index},
	{"size",tree_size},
	{0, 0}
};/*}}}*/

static const LuaLReg tree_meta[] = {/*{{{*/
	{"__gc", tree_gc},
	{"__tostring", tree_tostring},
	{0, 0}
};/*}}}*/

int luaopen_moonshine_tree(LuaState *L)/*{{{*/
{
	ms_lua_class_register(L, CLASS, tree_methods, tree_meta);
	return 1;
}/*}}}*/
