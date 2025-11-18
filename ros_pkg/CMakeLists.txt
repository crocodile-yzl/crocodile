cmake_minimum_required(VERSION 3.0.2)
project(ros_pkg)

## Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)

## Find catkin macros and libraries
## 合并重复的 find_package，避免冗余
find_package(catkin REQUIRED COMPONENTS
  geometry_msgs
  roscpp
  rospy
  sensor_msgs
  nav_msgs
  tf2_ros  # TF2 核心依赖
  tf       # tf::getYaw 等函数依赖
)

## System dependencies are found with CMake's conventions
# find_package(Boost REQUIRED COMPONENTS system)

###################################
## catkin specific configuration ##
###################################
catkin_package(
#  INCLUDE_DIRS include  # 若有自定义头文件目录，取消注释并添加路径
#  LIBRARIES ros_pkg     # 若有自定义库，取消注释并添加库名
  CATKIN_DEPENDS geometry_msgs roscpp rospy sensor_msgs nav_msgs tf2_ros tf
#  DEPENDS system_lib
)

###########
## Build ##
###########

## 头文件目录（无需重复配置，一次即可）
include_directories(
# include  # 若有自定义头文件目录，取消注释
  ${catkin_INCLUDE_DIRS}
)

## 编译 tf_node（你的原有节点）
add_executable(tf_node src/tf_node.cpp)
# 添加依赖（确保 ROS 消息/服务先编译）
add_dependencies(tf_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
# 链接库（catkin_LIBRARIES 已包含所有依赖库，无需单独链接 tf2_ros/tf）
target_link_libraries(tf_node
  ${catkin_LIBRARIES}
)
## 编译 tf_node（你的原有节点）
add_executable(try_node src/try_node.cpp)
# 添加依赖（确保 ROS 消息/服务先编译）
add_dependencies(try_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
# 链接库（catkin_LIBRARIES 已包含所有依赖库，无需单独链接 tf2_ros/tf）
target_link_libraries(try_node
  ${catkin_LIBRARIES}
)

# add_executable(position_node src/position_node.cpp)
# # 把 ${PROJECT_NAME} 改成 可执行文件名 vel_node，同时简化冗余依赖
# add_dependencies(position_node ${catkin_EXPORTED_TARGETS})
# target_link_libraries(position_node ${catkin_LIBRARIES})

#############
## Install ##
#############

# 可选：安装可执行文件到 ROS 环境（方便 rosrun 调用）
install(TARGETS
  tf_node
  try_node
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
