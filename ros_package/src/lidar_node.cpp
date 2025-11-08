#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <geometry_msgs/Twist.h>
ros::Publisher vel_pub; 
geometry_msgs::Twist vel_cmd;
void lid_callback(const sensor_msgs::LaserScan::ConstPtr& msg)
{ 
    float ld_angle=msg->ranges[90];
    if(ld_angle<=0.5)
    {  
        vel_cmd.linear.x=0;
        ROS_INFO("已到达指定位置");
        vel_pub.publish(vel_cmd);
    }
    
}
int main(int argc,char **argv)
{
    ros::init(argc,argv,"lidar_node");
    ros::NodeHandle nh;
    ros::Subscriber ld_sub =nh.subscribe("/scan",10,lid_callback);
    vel_pub = nh.advertise<geometry_msgs::Twist>("/cmd_vel",10);
    ros::spin();

    return 0;
}