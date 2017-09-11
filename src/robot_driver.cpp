#include "ros/ros.h"
  
#include "math.h"
#include "std_msgs/Float32MultiArray.h"
#include "std_msgs/String.h"
#include "geometry_msgs/PoseStamped.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/JointState.h"
#include "sensor_msgs/Joy.h"

#include "tf/transform_listener.h"


#include <string>
#include <iostream>
#include <sstream>

float input[3]={0};
bool goal_mode=false;

ros::Publisher twist_pub;
void twist_publish(float *input){
	geometry_msgs::Twist twist_data;
	twist_data.linear.x=input[0];
	twist_data.linear.y=input[1];
	twist_data.angular.z=input[2];
	twist_pub.publish(twist_data);
	//printf("x:%f, y:%f, rz:%f\n",input[0],input[1],input[2]);
}
void twist_callback(const geometry_msgs::Twist& twist_msg){
	input[0]=twist_msg.linear.x;
	input[1]=twist_msg.linear.y;
	input[2]=twist_msg.angular.z;
}
void ctrl_callback(const std_msgs::String& ctrl_msg){
	goal_mode=true;
}
float normal_rad(float value){
	if(value>0)return fmod(value,3.1415);
	else return -fmod(-value,3.1415);
}
int main(int argc, char **argv)
{
	ros::init(argc, argv, "robot_driver");
	ros::NodeHandle n;
	
	twist_pub = n.advertise<geometry_msgs::Twist>("/twist", 1000); 
	ros::Subscriber twist_sub   = n.subscribe("/robot_twist", 10, twist_callback); 
	ros::Subscriber command_sub = n.subscribe("/ctrl_command", 10, ctrl_callback); 

	tf::TransformListener tflistener;

	ros::Rate loop_rate(20); 
	while (ros::ok()){
		if(goal_mode){
			geometry_msgs::PoseStamped source_pose;
			source_pose.header.frame_id="goal_link";
			source_pose.pose.orientation.w=1.0;
			geometry_msgs::PoseStamped target_pose;
			tflistener.waitForTransform("base_link", "goal_link", ros::Time(0), ros::Duration(1.0));
			tflistener.transformPose("base_link",ros::Time(0),source_pose,"goal_link",target_pose);

			//printf("P X:%f, Y:%f, Z:%f\n",target_pose.pose.position.x,target_pose.pose.position.y,target_pose.pose.position.z);
			float target_x=target_pose.pose.position.x;
			float target_y=target_pose.pose.position.y;

			float input2[3]={0};
			float distance=sqrt(target_x*target_x+target_y*target_y);
			float alpha=atan2(target_y,target_x);
			//float beta=alpha;
			if(distance<0.5){
				input2[0]=cos(alpha)*distance;
				input2[1]=sin(alpha)*distance;
			}
			else{
				input2[0]=cos(alpha)*0.5;
				input2[1]=sin(alpha)*0.5;
			}
			//if(beta<-0.1)input2[2]=1.0;
			//else if(beta>0.1)input2[2]=-1.0;
			//else input2[2]=-beta;

			//if(distance<0.1 && fabsf(beta)<0.1)goal_mode=false;
			if(distance<0.1)goal_mode=false;
			twist_publish(input2);
		}
		else{
			twist_publish(input);
		}
		ros::spinOnce();
		loop_rate.sleep();
	} 
 	return 0;
}

