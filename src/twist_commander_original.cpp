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

float input[3]={0};
float position[3]={0};
bool goal_mode=false;
float goal[3]={0};

ros::Publisher twist_pub;
void twist_publish(float *input){
	geometry_msgs::Twist twist_data;
	twist_data.linear.x=input[0];
	twist_data.linear.y=input[1];
	twist_data.angular.z=input[2];
	twist_pub.publish(twist_data);
	printf("x:%f, y:%f, rz:%f\n",input[0],input[1],input[2]);
}
void joy_callback(const sensor_msgs::Joy& joy_msg){
	input[0]=joy_msg.axes[1];
	input[1]=joy_msg.axes[0];
	input[2]=joy_msg.axes[2]*2.0;
}
void goal_callback(const geometry_msgs::PoseStamped& goal_msg){
	goal[0]=goal_msg.pose.position.x;
	goal[1]=goal_msg.pose.position.y;
	if(goal_msg.pose.orientation.w>0)goal[2]=asin(goal_msg.pose.orientation.z)*2;
	else goal[2]=-asin(goal_msg.pose.orientation.z)*2;
	goal_mode=true;
	//printf("goal x:%f, y:%f zr:%f\n",goal[0],goal[1],goal[2]);
	printf("zr:%f, %f\n",goal_msg.pose.orientation.z,goal[2]);	
}
void robot_pose_callback(const std_msgs::Float32MultiArray& robot_pose_msg){
	position[0]=robot_pose_msg.data[0];
	position[1]=robot_pose_msg.data[1];
	position[2]=robot_pose_msg.data[2];
}

float normal_rad(float value){
	if(value>0)return fmod(value,3.1415);
	else return -fmod(-value,3.1415);
}
int main(int argc, char **argv)
{
	ros::init(argc, argv, "twist_commander");
	ros::NodeHandle n;
	
	twist_pub = n.advertise<geometry_msgs::Twist>("/twist", 1000); 
	ros::Subscriber joy_sub   = n.subscribe("/joy", 10, joy_callback); 
	ros::Subscriber goal_sub  = n.subscribe("/move_base_simple/goal", 10, goal_callback); 
	ros::Subscriber twist_sub= n.subscribe("/robot_pose", 10, robot_pose_callback);

	ros::Rate loop_rate(20); 
	while (ros::ok()){
		if(goal_mode){
			float input2[3]={0};
			float distance=sqrt((goal[0]-position[0])*(goal[0]-position[0])+(goal[1]-position[1])*(goal[1]-position[1]));
			float alpha=atan2(goal[1]-position[1],goal[0]-position[0])-position[2];
			float beta=normal_rad(position[2]-goal[2]);
			//printf("post:%f goal:%f\n",position[2],goal[2]);
			//printf("dist:%f alph:%f beta:%f\n",distance,alpha,beta);
			if(distance<0.5){
				input2[0]=cos(alpha)*distance;
				input2[1]=sin(alpha)*distance;
			}
			else{
				input2[0]=cos(alpha)*0.5;
				input2[1]=sin(alpha)*0.5;
			}
			if(beta<-0.1)input2[2]=1.0;
			else if(beta>0.1)input2[2]=-1.0;
			else input2[2]=-beta;

			if(distance<0.1 && fabsf(beta)<0.1)goal_mode=false;
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

