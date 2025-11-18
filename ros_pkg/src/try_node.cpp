#include <ros/ros.h>
#include <tf2_ros/transform_listener.h>
#include <geometry_msgs/TransformStamped.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <tf/transform_datatypes.h>
#include <cmath>

// 全局变量
double tb31_yaw = 0.0;          // tb3_1的自身偏航角
double relative_x = 0.0;         // tb3_1相对于tb3_0的x距离
double relative_y = 0.0;         // tb3_1相对于tb3_0的y距离
const double POS_EPS = 0.08;     // 位置容差
const double ANG_EPS = 0.08;     // 角度容差
const double PI = M_PI;
const double LINEAR_SPEED = 0.2; // 线速度
const double ANGULAR_SPEED = 0.3;// 角速度
const double FOLLOW_DIST = 0.5;  // 期望跟随距离（tb3_1在tb3_0前方0.5m）

// 订阅tb3_1的里程计（带命名空间）
void tb31_odom_callback(const nav_msgs::Odometry::ConstPtr& msg)
{
    tf::Quaternion quat;
    tf::quaternionMsgToTF(msg->pose.pose.orientation, quat);
    tb31_yaw = tf::getYaw(quat);
    tb31_yaw = atan2(sin(tb31_yaw), cos(tb31_yaw)); // 归一化到[-π, π]
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "tf_follow_node");
    ros::NodeHandle nh;

    // TF监听器
    tf2_ros::Buffer tf_buffer;
    tf2_ros::TransformListener tf_listener(tf_buffer);

    // 订阅与发布：明确带命名空间的话题
    ros::Subscriber odom_sub = nh.subscribe("/tb3_1/odom", 10, tb31_odom_callback);
    ros::Publisher vel_pub = nh.advertise<geometry_msgs::Twist>("/tb3_1/cmd_vel", 10);

    geometry_msgs::Twist vel_msg;
    ros::Rate rate(10);
    geometry_msgs::TransformStamped relative_tf;
    bool tf_valid = false;

    while (ros::ok())
    {
        // 获取TF变换：使用带命名空间的坐标系（tb3_0和tb3_1的base_footprint）
        try
        {
            relative_tf = tf_buffer.lookupTransform(
                "tb3_0/base_footprint",  // 目标坐标系（tb3_0的基座）
                "tb3_1/base_footprint",  // 源坐标系（tb3_1的基座）
                ros::Time(0),
                ros::Duration(0.5)
            );
            relative_x = relative_tf.transform.translation.x;
            relative_y = relative_tf.transform.translation.y;
            tf_valid = true;
            ROS_INFO_THROTTLE(1.0, "相对位置：x=%.2fm, y=%.2fm", relative_x, relative_y);
        }
        catch (tf2::TransformException &ex)
        {
            ROS_WARN_THROTTLE(1.0, "TF获取失败: %s", ex.what());
            tf_valid = false;
            vel_msg.linear.x = 0.0;
            vel_msg.angular.z = 0.0;
            vel_pub.publish(vel_msg);
            rate.sleep();
            ros::spinOnce();
            continue;
        }

        // 控制逻辑（与之前一致）
        if (tf_valid)
        {
            vel_msg.linear.x = 0.0;
            vel_msg.angular.z = 0.0;

            double error_y = relative_y - 0.0;
            double error_x = relative_x - FOLLOW_DIST;

            // 优先横向对齐（y方向）
            if (fabs(error_y) > POS_EPS)
            {
                double target_angle = atan2(-error_y, 0);
                double angle_error = target_angle - tb31_yaw;
                angle_error = atan2(sin(angle_error), cos(angle_error));

                if (fabs(angle_error) > ANG_EPS)
                {
                    vel_msg.angular.z = ANGULAR_SPEED * (angle_error > 0 ? 1 : -1);
                }
            }
            // 横向对齐后，修正纵向距离（x方向）
            else if (fabs(error_x) > POS_EPS)
            {
                double target_angle = atan2(-error_x, 0);
                double angle_error = target_angle - tb31_yaw;
                angle_error = atan2(sin(angle_error), cos(angle_error));

                if (fabs(angle_error) > ANG_EPS)
                {
                    vel_msg.angular.z = ANGULAR_SPEED * (angle_error > 0 ? 1 : -1);
                }
                else
                {
                    vel_msg.linear.x = LINEAR_SPEED * (-error_x > 0 ? 1 : -1);
                }
            }
            else
            {
                ROS_INFO_THROTTLE(1.0, "已到达跟随位置！");
            }

            vel_pub.publish(vel_msg);
        }

        ros::spinOnce();
        rate.sleep();
    }

    return 0;
}
