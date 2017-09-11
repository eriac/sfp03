#include "ros/ros.h"
  
#include "math.h"
#include "std_msgs/Float32MultiArray.h"
#include "std_msgs/String.h"
#include "geometry_msgs/PoseStamped.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/JointState.h"
#include "sensor_msgs/Joy.h"


#include <string>
#include <iostream>
#include <sstream>

float input[6]={0};
ros::Publisher robot_twist_pub;
ros::Publisher ctrl_twist_pub;
ros::Publisher ctrl_command_pub;

void robot_twist_publish(float *input){
	geometry_msgs::Twist twist_data;
	twist_data.linear.x =input[0];
	twist_data.linear.y =input[1];
	twist_data.angular.z=input[2];
	robot_twist_pub.publish(twist_data);
	printf("x:%f, y:%f, rz:%f\n",input[0],input[1],input[2]);
}
void ctrl_twist_publish(float *input, bool pub){
	geometry_msgs::Twist twist_data;
	twist_data.linear.x =input[0];
	twist_data.linear.y =input[1];
	twist_data.angular.y=input[2];
	twist_data.angular.z=input[3];
	twist_data.linear.z =input[4];
	twist_data.angular.x=input[5];

	ctrl_twist_pub.publish(twist_data);
	//printf("x:%f, y:%f, rz:%f\n",input[0],input[1],input[2]);
	if(pub){
		std_msgs::String str_msg;
		str_msg.data="pub";
		ctrl_command_pub.publish(str_msg);
	}
}

void joy_callback(const sensor_msgs::Joy& joy_msg){
	input[0]=joy_msg.axes[1];
	input[1]=joy_msg.axes[0];
	input[2]=joy_msg.axes[2];
	input[3]=joy_msg.axes[3];
	if(joy_msg.buttons[4])input[4]=1.0;
	else if(joy_msg.buttons[6])input[4]=-1.0;
	else input[4]=0.0;
	if(joy_msg.buttons[5])input[5]=1.0;
	else if(joy_msg.buttons[7])input[5]=-1.0;
	else input[5]=0.0;

	static std::string mode="Robot";
	if(joy_msg.buttons[12])mode="Robot";
	else if(joy_msg.buttons[13])mode="Ctrl";

	static bool last_push=false;
	bool now_push=false;
	bool press=false;
	if(joy_msg.buttons[14])now_push=true;
	press=now_push && !last_push;
	last_push=now_push;
	
	if(mode=="Robot"){
		input[2]*=2.0;
		robot_twist_publish(input);
	}
	else if(mode=="Ctrl"){
		input[2]*=2.0;
		ctrl_twist_publish(input,press);
	}
}
int main(int argc, char **argv)
{
	ros::init(argc, argv, "twist_commander");
	ros::NodeHandle n;
	
	robot_twist_pub  = n.advertise<geometry_msgs::Twist>("/robot_twist", 1000); 
	ctrl_twist_pub   = n.advertise<geometry_msgs::Twist>("/ctrl_twist", 1000); 
	ctrl_command_pub = n.advertise<std_msgs::String>("/ctrl_command", 1000); 
	ros::Subscriber joy_sub   = n.subscribe("/joy", 10, joy_callback); 

	ros::Rate loop_rate(20); 
	while (ros::ok()){		
		ros::spinOnce();
		loop_rate.sleep();
	} 
 	return 0;
}

