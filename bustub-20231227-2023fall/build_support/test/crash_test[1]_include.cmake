if(EXISTS "/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/test/crash_test[1]_tests.cmake")
  include("/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/test/crash_test[1]_tests.cmake")
else()
  add_test(crash_test_NOT_BUILT crash_test_NOT_BUILT)
endif()