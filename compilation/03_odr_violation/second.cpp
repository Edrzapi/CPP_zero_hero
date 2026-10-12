// second.cpp: a DIFFERENT definition of the same inline function.
// This breaks the One Definition Rule. See main.cpp.

inline int magic_number() { return 2; }

int value_seen_by_second() { return magic_number(); }
