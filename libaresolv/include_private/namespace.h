// NetBSD uses __weak_alias on a lot of functions. We don't want that.
#if defined(__weak_alias)
#undef __weak_alias
#endif