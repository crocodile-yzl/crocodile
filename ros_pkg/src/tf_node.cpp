#include <ros/ros.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/buffer.h>
#include <geometry_msgs/PointStamped.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <geometry_msgs/TransformStamped.h>
#include <tf2/exceptions.h>
#include <geometry_msgs/Twist.h>
#include <cmath> 
#include <nav_msgs/Odometry.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2/LinearMath/Quaternion.h>  
#include <tf2/LinearMath/Matrix3x3.h> 


// 里程计回调函数（tb3_0）：订阅里程计，实时广播TF
void tb3_0Callback(const nav_msgs::Odometry::ConstPtr& odom) {
    static tf2_ros::TransformBroadcaster br;
    geometry_msgs::TransformStamped tfstamped;

    tfstamped.header.frame_id = "world";
    tfstamped.header.stamp = ros::Time::now();
    tfstamped.child_frame_id = "/tb3_0/base_footprint"; // 与查询的坐标系一致
    
    // 从里程计获取实时位置
    tfstamped.transform.translation.x = odom->pose.pose.position.x;
    tfstamped.transform.translation.y = odom->pose.pose.position.y;
    tfstamped.transform.translation.z = odom->pose.pose.position.z;
    
    // 从里程计获取实时姿态（四元数转换，确保姿态准确）
    tf2::Quaternion q;
    tf2::fromMsg(odom->pose.pose.orientation, q);
    double roll, pitch, yaw;
    tf2::Matrix3x3(q).getRPY(roll, pitch, yaw);
    tf2::Quaternion qtn;
    qtn.setRPY(roll, pitch, yaw);
    
    tfstamped.transform.rotation.x = qtn.getX();
    tfstamped.transform.rotation.y = qtn.getY();
    tfstamped.transform.rotation.z = qtn.getZ();
    tfstamped.transform.rotation.w = qtn.getW();
    

    br.sendTransform(tfstamped);
}

void  tb3_1Callback(const nav_msgs::Odometry::ConstPtr& odom) {
    static tf2_ros::TransformBroadcaster tf_broadcaster;
    geometry_msgs::TransformStamped ts;
    ts.header.frame_id = "world";
    ts.header.stamp = ros::Time::now();
    ts.child_frame_id = "tb3_1/base_footprint";
    
    ts.transform.translation.x = odom->pose.pose.position.x;
    ts.transform.translation.y = odom->pose.pose.position.y;
    ts.transform.translation.z = odom->pose.pose.position.z;
    
    tf2::Quaternion q;
    tf2::fromMsg(odom->pose.pose.orientation, q);
    double roll, pitch, yaw;
    tf2::Matrix3x3(q).getRPY(roll, pitch, yaw);
    tf2::Quaternion qtn;
    qtn.setRPY(roll, pitch, yaw);
    
    ts.transform.rotation.x = qtn.getX();
    ts.transform.rotation.y = qtn.getY();
    ts.transform.rotation.z = qtn.getZ();
    ts.transform.rotation.w = qtn.getW();
    
    tf_broadcaster.sendTransform(ts);
}

int main(int argc, char**argv)
{
    ros::init(argc, argv, "tf_node"); 
    ros::NodeHandle nh;

    // 创建TF监听器（添加10秒缓冲，确保实时查询成功）
    tf2_ros::Buffer buffer(ros::Duration(10.0));
    tf2_ros::TransformListener sub(buffer);
    
    // 速度发布者
    ros::Publisher pub = nh.advertise<geometry_msgs::Twist>("/tb3_1/cmd_vel", 100);
    //订阅两个速度
    ros::Subscriber odom_sub_tb30 = nh.subscribe<nav_msgs::Odometry>("tb3_0/odom", 100, tb3_0Callback);
    ros::Subscriber odom_sub_tb31 = nh.subscribe<nav_msgs::Odometry>("tb3_1/odom", 100, tb3_1Callback);

    ros::Rate rate(10);
    while (ros::ok())
    {
        // 实时查询里程计驱动的TF，计算速度控制跟随
        try
        {   
            // 查询tb3_0相对于tb3_1的实时TF
            geometry_msgs::TransformStamped tb3_0Totb3_1 = buffer.lookupTransform("tb3_1/base_footprint","tb3_0/base_footprint",ros::Time(0));
            ROS_INFO("x=%3.f,y=%3.f",tb3_0Totb3_1.transform.translation.x,tb3_0Totb3_1.transform.translation.y);
            // 曼哈顿距离计算
            double manhattan_dist = fabs(tb3_0Totb3_1.transform.translation.x) + fabs(tb3_0Totb3_1.transform.translation.y);
            //double current_dist = sqrt(pow(tb3_0Totb3_1.transform.translation.x, 2) + pow(tb3_0Totb3_1.transform.translation.y, 2));

            geometry_msgs::Twist twist;
           if(manhattan_dist>0.5)
            {
            twist.linear.x = 0.1*sqrt(pow(tb3_0Totb3_1.transform.translation.x,2)+pow(tb3_0Totb3_1.transform.translation.y,2));
            twist.angular.z = 5*atan2(tb3_0Totb3_1.transform.translation.y,tb3_0Totb3_1.transform.translation.x);
            }
            else 
            {
            twist.linear.x=0;
            twist.angular.z=0;
            }

            // 发布速度指令
            pub.publish(twist);
        }
         catch(const std::exception& e)
        {
            // std::cerr << e.what() << '\n';
            ROS_INFO("错误提示:%s",e.what());
        }
        rate.sleep();
        ros::spinOnce(); 
    }
    return 0;
}