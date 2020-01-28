/// \file
/// \brief finds odometry of the robot and publishes it 

/// Publishers : odom
/// Subscribers : jointstate

#include "ros/ros.h"
#include "tf/transform_broadcaster.h"
#include "nav_msgs/Odometry.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/JointState.h"
#include"rigid2d/rigid2d.hpp"
#include"rigid2d/diff_drive.hpp"
#include"rigid2d/waypoints.hpp"


static double incoming_left_wheel;
static double incoming_right_wheel;
static double left_wheel = 0;
static double right_wheel =0;

void jt_callback(const sensor_msgs::JointState JT)
{
      incoming_left_wheel= JT.position[0];
      incoming_right_wheel= JT.position[1];

}

int main(int argc, char** argv)
{
      Twist2D starting_position;
      starting_position.v_x =5.5;
      starting_position.v_y =0;
      starting_position.w =0.0;
      DiffDrive turtle_odo(starting_position,0.5,0.05);
      Twist2D Vb;

      ros::init(argc, argv, "odometer");
      ros::NodeHandle n;
      ros::Publisher odom_pub = n.advertise<nav_msgs::Odometry>("odom", 1);

      ros::Subscriber joint_state_subsciber = n.subscribe("/joint_states", 1, jt_callback);
      tf::TransformBroadcaster odom_broadcaster;

      double x = 0.0;
      double y = 0.0;
      double th = 0.0;
      ros::Time current_time, last_time;
      current_time = ros::Time::now();
      last_time = ros::Time::now();
      ros::Rate r(1);
      while(n.ok())
      {
            ros::spinOnce();               // check for incoming messages
            current_time = ros::Time::now();
            left_wheel = incoming_left_wheel- left_wheel;
            right_wheel = incoming_right_wheel - right_wheel;
            turtle_odo.feedforward(left_wheel, right_wheel);

            WheelVelocities temp_wheel;
            temp_wheel.U1 = left_wheel;
            temp_wheel.U2 = right_wheel;


            left_wheel = incoming_left_wheel;
            right_wheel = incoming_right_wheel;

            Twist2D current_position;
            current_position = turtle_odo.pose();

            x = current_position.v_x;
            y = current_position.v_y;
            th = current_position.w;
            geometry_msgs::Quaternion odom_quat = tf::createQuaternionMsgFromYaw(th);   
            
            geometry_msgs::TransformStamped odom_trans;
            odom_trans.header.stamp = current_time;
            odom_trans.header.frame_id = "odom";
            odom_trans.child_frame_id = "base_link";
   
            odom_trans.transform.translation.x = x;
            odom_trans.transform.translation.y = y;
            odom_trans.transform.translation.z = 0.0;
            odom_trans.transform.rotation = odom_quat;
   
            odom_broadcaster.sendTransform(odom_trans);
   
            nav_msgs::Odometry odom;
            odom.header.stamp = current_time;
            odom.header.frame_id = "odom";
   
            odom.pose.pose.position.x = x;
            odom.pose.pose.position.y = y;
            odom.pose.pose.position.z = 0.0;
            odom.pose.pose.orientation = odom_quat;

            odom.child_frame_id = "base_link";
            Vb = turtle_odo.wheelsToTwist(temp_wheel);
            odom.twist.twist.linear.x = Vb.v_x;
            odom.twist.twist.linear.y = Vb.v_y;
            odom.twist.twist.angular.z = Vb.w;
   
            odom_pub.publish(odom);
   
           last_time = current_time;
           r.sleep();
     }
    
 
}