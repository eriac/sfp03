#include "ros/ros.h"
  
#include "math.h"  
#include "std_msgs/Float32MultiArray.h"
#include "geometry_msgs/PoseStamped.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/JointState.h"
#include "sensor_msgs/Joy.h"


#include <string>
#include <iostream>
#include <sstream>

float position[3]={0};
ros::Publisher  joint_pub;
void joint_publish(float *angle){
	sensor_msgs::JointState jointstate;
	jointstate.header.stamp = ros::Time::now();
	jointstate.name.resize(3);
	jointstate.position.resize(3);
	jointstate.header.stamp = ros::Time::now();
	jointstate.name[0]="robot_joint1";
	jointstate.name[1]="robot_joint2";
	jointstate.name[2]="robot_joint3";
	jointstate.position[0]=angle[0];
	jointstate.position[1]=angle[1];
	jointstate.position[2]=angle[2];
	joint_pub.publish(jointstate);
}
void robot_pose_callback(const std_msgs::Float32MultiArray& robot_pose_msg){
	position[0]=robot_pose_msg.data[0];
	position[1]=robot_pose_msg.data[1];
	position[2]=robot_pose_msg.data[2];
	joint_publish(position);
}
int main(int argc, char **argv)
{
	ros::init(argc, argv, "robot_disp");
	ros::NodeHandle n;
	
	joint_pub  = n.advertise<sensor_msgs::JointState>("/joint_states", 1000);	
	ros::Subscriber twist_sub= n.subscribe("/robot_pose", 10, robot_pose_callback); 
	
	ros::Rate loop_rate(20); 
	while (ros::ok()){
		ros::spinOnce();
		loop_rate.sleep();
	} 
 	return 0;
}

