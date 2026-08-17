cmake_minimum_required (VERSION 3.14.10)

project(car_system_pkg)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ROS2 ament依赖
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 子模块 add_subdirectory
add_subdirectory(modules/Basemodules/safe_queue_module)
add_subdirectory(modules/Basemodules/sensor_data_module)   # 底层传感器数据结构体，最先加载
add_subdirectory(modules/logger_module)
add_subdirectory(modules/Basemodules/memory_pool_module)
add_subdirectory(modules/Basemodules/cache_data_module)    # cache依赖safe_queue + sensor_data
add_subdirectory(modules/dispatcher_module)
add_subdirectory(modules/ros_node_module)


# ---------------- 业务可执行 主程序 main_exec ----------------
add_executable(main_exec
  src/main.cpp
)
target_link_libraries(main_exec
  cache_data_lib
  memory_pool_lib
  safe_queue_lib
  sensor_data_lib
  dispatcher_lib
  logger_lib
  ros_node_lib
)
ament_target_dependencies(main_exec
  rclcpp
  std_msgs
)

# ---------------- 订阅接收端 sub_receiver_exec ----------------
add_executable(sub_receiver_exec
  src/sub_main.cpp
)
target_link_libraries(sub_receiver_exec
  ros_node_lib
  sensor_data_lib
  memory_pool_lib
)
ament_target_dependencies(sub_receiver_exec
  rclcpp
  std_msgs
)

# ---------------- install 安装规则 ----------------
# 安装可执行文件
install(TARGETS
  main_exec
  sub_receiver_exec
  DESTINATION lib/${PROJECT_NAME}
)

# 安装内部静态库，对外导出target
install(TARGETS
  safe_queue_lib
  sensor_data_lib
  cache_data_lib
  memory_pool_lib
  dispatcher_lib
  logger_lib
  ros_node_lib
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_export_targets(export_${PROJECT_NAME})

# ---------------- GTest 单元测试 ----------------
if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  ament_add_gtest(test_safe_queue
    test/test_safe_queue.cpp
  )
  target_link_libraries(test_safe_queue
    safe_queue_lib
  )

  ament_add_gtest(test_logger
    test/test_logger.cpp
  )
  target_link_libraries(test_logger
    logger_lib
  )

  ament_add_gtest(test_memory_pool
    test/test_memory_pool.cpp
  )
  target_link_libraries(test_memory_pool
    memory_pool_lib
  )

endif()

ament_package()

