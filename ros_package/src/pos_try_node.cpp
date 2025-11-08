#include <stdio.h>
#include <iostream>
#include <ros/ros.h>
#include <nav_msgs/Odometry.h>
#include <tf/transform_datatypes.h>
#include <sensor_msgs/LaserScan.h>
#include <geometry_msgs/Twist.h>
#include <tf/tf.h>
//sensor_msgs::LaserScan *lidar;
float ld_angle=0;
ros::Publisher vel_pub;
geometry_msgs::Twist vel_cmd;
void lid_callback(const sensor_msgs::LaserScan::ConstPtr& msg)
{   
    ld_angle=msg->ranges[0];
// lidar.ranges = &ld_angle;
    if(ld_angle<=0.5)
    {  
        vel_cmd.linear.x=0;
        ROS_INFO("已到达指定位置");
        vel_pub.publish(vel_cmd);
    }
}
void pos_callback(const nav_msgs::Odometry::ConstPtr& msg){
    double x = msg->pose.pose.position.x;
    double y = msg->pose.pose.position.y;
    double z = msg->pose.pose.position.z;
        tf::Quaternion quat;
        tf::quaternionMsgToTF(msg->pose.pose.orientation, quat);
        double yaw = tf::getYaw(quat);
    ROS_INFO("机器人位置: x=%.2f, y=%.2f, z=%.2f | 偏航角: %.2f rad", x, y, z, yaw);
    geometry_msgs::Twist vel_cmd;
if (ld_angle>0.5)
{
  if (x>=0.5)
    {
        vel_cmd.linear.x=0.0;
        vel_cmd.angular.x = 0.0;
        vel_cmd.angular.y = 0.0;
        vel_cmd.angular.z = -0.2;
        if (yaw <= -1.5)
        {
            vel_cmd.angular.z=0;
            vel_cmd.linear.x=0.2;
        }
        
    }else vel_cmd.linear.x=0.2;
    vel_pub.publish(vel_cmd);
}
}
int main(int argc,char **argv)
{
    ros::init(argc,argv,"position_node");
     ros::NodeHandle nh;
    ros::Subscriber pos_sub=nh.subscribe("/odom",10,pos_callback);
    vel_pub = nh.advertise<geometry_msgs::Twist>("/cmd_vel",10);
    ros::Subscriber ld_sub =nh.subscribe("/scan",10,lid_callback);
    ros::spin();

    return 0;

}
