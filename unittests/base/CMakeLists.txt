add_custom_target(ModuleBaseUnittests)
set_target_properties(ModuleBaseUnittests PROPERTIES FOLDER "ModuleBaseUnittests")

set(PDK_LANG_TEST_SRCS)
pdk_add_files(PDK_LANG_TEST_SRCS
    lang/CharacterTest.cpp)

pdk_add_unittest(ModuleBaseUnittests LangTest ${PDK_LANG_TEST_SRCS})

set(PDK_OS_THREAD_TEST_SRCS)
pdk_add_files(PDK_OS_THREAD_TEST_SRCS
    os/thread/AtomicIntTest.cpp
    os/thread/AtomicIntegerTest.cpp
    os/thread/AtomicPointerTest.cpp)

pdk_add_unittest(ModuleBaseUnittests OsThreadTest ${PDK_OS_THREAD_TEST_SRCS})

set(PDK_DS_TEST_SRCS)
pdk_add_files(PDK_DS_TEST_SRCS
    ds/arraydata/SimpleVector.h
    ds/arraydata/ArrayDataTest.cpp
    ds/ByteArrayTest.cpp
    ds/ByteArrayMatcherTest.cpp)

pdk_add_unittest(ModuleBaseUnittests DsTest ${PDK_DS_TEST_SRCS})
