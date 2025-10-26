set(target metric_accumulator_test)

add_executable(${target}
    tests/average_accumulator.cpp
    tests/categorical_accumulator.cpp
    tests/sum_average_accumulator.cpp
)

target_link_libraries(${target}
    PRIVATE
        GTest::GTest
        GTest::Main
        metric_accumulator
        function
        file
)

add_test(NAME ${target} COMMAND ${target})