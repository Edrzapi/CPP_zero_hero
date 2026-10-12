# memory: stack vs heap, new/delete, RAII, smart pointers,
# rule of 0/3/5, copy vs move, move-only types

zh_add_example(memory 01_stack_and_heap         01_stack_and_heap.cpp)
zh_add_example(memory 02_raii                   02_raii.cpp)
zh_add_example(memory 03_smart_pointers         03_smart_pointers.cpp)
zh_add_example(memory 04_rule_of_zero_three_five 04_rule_of_zero_three_five.cpp)
zh_add_example(memory 05_move_semantics         05_move_semantics.cpp)

zh_add_topic_runner(memory)
