#include "ros/ros.h"
  
#include "math.h"  
#include "geometry_msgs/PoseStamped.h"
#include "sensor_msgs/JointState.h"
#include "sensor_msgs/Joy.h"


#include <string>
#include <iostream>
#include <sstream>

float input[3]={0};
float position[3]={0};
bool goal_mode=false;
float goal[3]={0};
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
void robot_tick(float *input, float dt){
	position[0]+=(cos(position[2])*input[0]-sin(position[2])*input[1])*dt;
	position[1]+=(sin(position[2])*input[0]+cos(position[2])*input[1])*dt;
	position[2]+=input[2]*dt;
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
float normal_rad(float value){
	if(value>0)return fmod(value,3.1415);
	else return -fmod(-value,3.1415);
}
int main(int argc, char **argv)
{
	ros::init(argc, argv, "joy_driver");
	ros::NodeHandle n;
	
	joint_pub  = n.advertise<sensor_msgs::JointState>("/joint_states", 1000);	
	ros::Subscriber joy_sub     = n.subscribe("/joy", 10, joy_callback); 
	ros::Subscriber goal_sub     = n.subscribe("/move_base_simple/goal", 10, goal_callback); 
	
	ros::Rate loop_rate(20); 
	while (ros::ok()){
		if(goal_mode){
			float input2[3]={0};
			float distance=sqrt((goal[0]-position[0])*(goal[0]-position[0])+(goal[1]-position[1])*(goal[1]-position[1]));
			float alpha=atan2(goal[1]-position[1],goal[0]-position[0])-position[2];
			float beta=normal_rad(position[2]-goal[2]);
			//printf("post:%f goal:%f\n",position[2],goal[2]);
			//printf("dist:%f alph:%f beta:%f\n",distance,alpha,beta);
			input2[0]=cos(alpha)*0.5;
			input2[1]=sin(alpha)*0.5;
			if(beta<-0.1)input2[2]=1.0;
			else if(beta>0.1)input2[2]=-1.0;
			if(distance<0.1 && fabsf(beta)<0.15)goal_mode=false;
			robot_tick(input2,0.05);			
		}
		else{
			robot_tick(input,0.05);
		}
		joint_publish(position);
		ros::spinOnce();
		loop_rate.sleep();
	} 
 	return 0;
}

