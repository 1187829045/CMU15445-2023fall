if(EXISTS "/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/test/lru_replacer_test[1]_tests.cmake")
  include("/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/test/lru_replacer_test[1]_tests.cmake")
else()
  add_test(lru_replacer_test_NOT_BUILT lru_replacer_test_NOT_BUILT)
endif()