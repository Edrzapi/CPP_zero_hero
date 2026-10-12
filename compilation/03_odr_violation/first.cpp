// first.cpp: one definition of the inline function magic_number().
// Part of the ODR violation demo; see main.cpp.

inline int magic_number() { return 1; }

int value_seen_by_first() { return magic_number(); }
