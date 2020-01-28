/// \file
/// \brief This file makes a turtle travel in a pentagon, using a feedforward
/// strategy of control. It also prints the error between its expected position and 
/// the actual position, and also plots it
/// PARAMETERS:
/// x positions 
/// y positions 

/// PUBLISHES:
///     vel_pub (cmd_vel): publishes velocity commands to move the turtle 
///     err_pub (pose_err): this topic takes in error in actual and expected position of the turtle
/// SUBSCRIBES:
///     sub (pose): reads actual position data of the turtle
///     sub (odom): odometry readings from the /odom topic 
/// SERVICES:
///     client2 (SetPen): can be used to change color and transperency of turtle marker
///     client   (TeleportAbsolute) used to teleport the turtle to desired point and orientation
///     

#include "ros/ros.h"
#include "std_msgs/String.h"
#include "geometry_msgs/Twist.h"
#include <iostream>
#include "nav_msgs/Odometry.h"
#include <turtlesim/TeleportAbsolute.h>
#include <turtlesim/SetPen.h>
#include<std_srvs/Empty.h>
#include <turtlesim/Pose.h> 
#include "tsim/PoseError.h"
#include <math.h>
#include <iostream>
#include<iosfwd> // contains forward definitions for iostream objects
#include"rigid2d/rigid2d.hpp"
#include"rigid2d/diff_drive.hpp"
#include"rigid2d/waypoints.hpp"
#include "tf/transform_broadcaster.h"
#include <vector>

using namespace rigid2d;
geometry_msgs::Twist msg;
tsim::PoseError er;
turtlesim::Pose pose;
nav_msgs::Odometry odom2;
double roll, pitch, yaw;
   

namespace turtle_pose{
    float x = 0.0;
    float y = 0.0;
    float theta = 0.0;
}
namespace turtle_odom_pose{
    float x = 0.0;
    float y = 0.0;
    float theta = 0.0;
    geometry_msgs::Quaternion quat; 
    
}
/// \brief is called at every ros::spin. passes all information of that particular
/// topic
/// \returns odometry information

void pose_odom_Callback(const nav_msgs::Odometry &odom_pose){
   turtle_odom_pose::x = odom_pose.pose.pose.position.x ;
   turtle_odom_pose::y = odom_pose.pose.pose.position.y;
   turtle_odom_pose::quat = odom_pose.pose.pose.orientation;

   double quatx= odom_pose.pose.pose.orientation.x;
   double quaty= odom_pose.pose.pose.orientation.y;
   double quatz= odom_pose.pose.pose.orientation.z;
   double quatw= odom_pose.pose.pose.orientation.w;
   tf::Quaternion q(quatx, quaty, quatz, quatw);
   tf::Matrix3x3 m(q);
   m.getRPY(roll, pitch, yaw);
//    ROS_INFO_STREAM(yaw);
//    ROS_INFO_STREAM(turtle_odom_pose::y);
}

/// \brief is called at every ros::spin. passes all information of that particular
/// topic
/// \returns turtle_pose

void poseCallback(const turtlesim::Pose &pose){
   turtle_pose::x = pose.x;
   turtle_pose::y = pose.y;
   turtle_pose::theta = pose.theta;

}

/// \brief using the turn and move strategy, this method turns the bot till 
/// the desired orientation is achieved
/// inputs: ang_rotation and publisher
/// \returns nothing
void move_angular(float ang_speed, ros::Publisher vel_pub)
{
    ros::spinOnce();    
    if(ang_speed ==0 )
    {
        msg.linear.x = 0;
        msg.angular.z = 0.1;
    while(abs(yaw - ang_speed)>0.12)
        {
            ros::spinOnce();    
            vel_pub.publish(msg);
        }
    msg.angular.z = 0;
    vel_pub.publish(msg);    
    }

    else
    {
    msg.linear.x = 0;
    msg.angular.z = ang_speed/10;
    std::cout<<"speed = "<< msg.angular.z <<"\n";
    while(abs(yaw - ang_speed)>0.05)
    {
        ros::spinOnce();    
        vel_pub.publish(msg);
    }
    msg.angular.z = 0;
    vel_pub.publish(msg);
    }
}


/// \brief using the turn and move strategy, this method moves the bot until 
/// the desired distance/position is achieved
/// inputs: linear distance , publisher, current x,y positions
/// \returns nothing
void move_linear(float linear_speed, ros::Publisher vel_pub, double x_p, double y_p)
{
    ros::spinOnce();    
    msg.linear.x = linear_speed/10;
    msg.angular.z = 0;

    while(abs(sqrt((turtle_odom_pose::x-x_p)*(turtle_odom_pose::x-x_p) + (turtle_odom_pose::y-y_p)*(turtle_odom_pose::y-y_p)) - linear_speed) >0.1)
    {
        ros::spinOnce();    
        vel_pub.publish(msg);
    }
    msg.linear.x = 0;
    vel_pub.publish(msg);
}

int main(int argc, char **argv)
{
    Waypoints whereto;
    Twist2D iamhere;    

    std::vector<double> x_cors = {0.5+5,0.8090+5,0.0+5,-0.8090+5,-0.5+5,0.5+5};
    std::vector<double>y_cors = {0.0, 0.9511, 1.5389, 0.9511, 0.0, 0.0};
    ros::init(argc,argv,"turtle_way");
    ros::NodeHandle n;

    ros::Publisher vel_pub = n.advertise<geometry_msgs::Twist>("turtle1/cmd_vel", 1);
    ros::Publisher err_pub = n.advertise<tsim::PoseError>("turtle1/pose_err", 1);

    ros::Subscriber sub = n.subscribe("turtle1/pose", 1, poseCallback);
    ros::Subscriber odom_sub = n.subscribe("/odom", 1, pose_odom_Callback);
    ros::service::waitForService("spawn");

    ros::ServiceClient client2 = n.serviceClient<turtlesim::SetPen>("turtle1/set_pen");
    turtlesim::SetPen srv2;
    srv2.request.r =0;
    srv2.request.g =0;
    srv2.request.b =0;
    srv2.request.width =1;
    srv2.request.off =1;
    client2.call(srv2);
    ros::ServiceClient client = n.serviceClient<turtlesim::TeleportAbsolute>("turtle1/teleport_absolute");
    turtlesim::TeleportAbsolute srv;
    srv.request.x =4;
    srv.request.y =2;
    srv.request.theta =0;
    client.call(srv);
    srv2.request.off =0;
    client2.call(srv2);

    ros::Rate rate(60);

    while (ros::ok())
    {
        unsigned int i =0;    
        for(i = 0; i<(x_cors.size()-1); i ++)
            {
                Vector2D temp_t;
                temp_t.x = x_cors[i+1];
                temp_t.y = y_cors[i+1];
                whereto.updatePose(temp_t);    
                iamhere.v_x = x_cors[i];
                iamhere.v_y = y_cors[i];
                iamhere.w=yaw;
  
                Twist2D whatsthespeed = whereto.nextWaypoint(iamhere);
                move_angular(whatsthespeed.w, vel_pub);
                move_linear(whatsthespeed.v_x, vel_pub, iamhere.v_x, iamhere.v_y);

                er.x_error = turtle_pose::x - turtle_odom_pose::x;
                er.y_error = turtle_pose::y - turtle_odom_pose::y;
                er.theta_error = turtle_pose::theta - turtle_odom_pose::theta;

                err_pub.publish(er);

    }
    i = 0;
    rate.sleep();
    }   
}
