#include <stdio.h>
#include <iostream>
#include <ros/ros.h>
#include <nav_msgs/Odometry.h>
#include <tf/transform_datatypes.h>
#include <geometry_msgs/Twist.h>
#include <tf/tf.h>

ros::Publisher vel_pub;
void pos_callback(const nav_msgs::Odometry::ConstPtr& msg){
    double x = msg->pose.pose.position.x;
    double y = msg->pose.pose.position.y;
    double z = msg->pose.pose.position.z;
        tf::Quaternion quat;
        tf::quaternionMsgToTF(msg->pose.pose.orientation, quat);
        double yaw = tf::getYaw(quat);
    ROS_INFO("机器人位置: x=%.2f, y=%.2f, z=%.2f | 偏航角: %.2f rad", x, y, z, yaw);
    geometry_msgs::Twist vel_cmd;
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
int main(int argc,char **argv)
{
    ros::init(argc,argv,"position_node");

    ros::NodeHandle nh;
    ros::Subscriber pos_sub=nh.subscribe("/odom",10,pos_callback);
    vel_pub = nh.advertise<geometry_msgs::Twist>("/cmd_vel",10);
    ros::spin();

    return 0;

}
