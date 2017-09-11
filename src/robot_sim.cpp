#include "ros/ros.h"
  
#include "math.h"  
#include <stdlib.h>
#include "std_msgs/Float32MultiArray.h"
#include "geometry_msgs/PoseStamped.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/JointState.h"
#include "sensor_msgs/Joy.h"

#include <tf/transform_broadcaster.h>

#include <string>
#include <iostream>
#include <sstream>

float input_twist[3]={0};
float position[3]={0};
float velocity[3]={0};
ros::Publisher  robot_pose_pub;
void robot_pose_publish(float *position){
	static tf::TransformBroadcaster br;
	tf::Transform transform;
	transform.setOrigin( tf::Vector3(position[0], position[1], 0.0) );
	tf::Quaternion q;
	q.setRPY(0, 0, position[2]);
	transform.setRotation(q);
	br.sendTransform(tf::StampedTransform(transform, ros::Time::now(), "world", "base_link"));

}
/*void robot_tick(float *input, float dt){
	position[0]+=(cos(position[2])*input[0]-sin(position[2])*input[1])*dt;
	position[1]+=(sin(position[2])*input[0]+cos(position[2])*input[1])*dt;
	position[2]+=input[2]*dt;
}*/
float sim_rand(){
	float min=0.7;
	float max=1.3;
	return min + (float)(rand()*(max-min+1.0)/(1.0+RAND_MAX));
}
void robot_tick(float *input, float dt){
	velocity[0]+=(input[0]-velocity[0])*2.0*dt*sim_rand();
	velocity[1]+=(input[1]-velocity[1])*2.0*dt*sim_rand();
	velocity[2]+=(input[2]-velocity[2])*2.0*dt*sim_rand();

	position[0]+=(cos(position[2])*velocity[0]-sin(position[2])*velocity[1])*dt;
	position[1]+=(sin(position[2])*velocity[0]+cos(position[2])*velocity[1])*dt;
	position[2]+=velocity[2]*dt;
}
void twist_callback(const geometry_msgs::Twist& twist_msg){
	input_twist[0]=twist_msg.linear.x;
	input_twist[1]=twist_msg.linear.y;
	input_twist[2]=twist_msg.angular.z;
}
float normal_rad(float value){
	if(value>0)return fmod(value,3.1415);
	else return -fmod(-value,3.1415);
}
int main(int argc, char **argv)
{
	ros::init(argc, argv, "robot_sim");
	ros::NodeHandle n;
	
	robot_pose_pub  = n.advertise<std_msgs::Float32MultiArray>("/robot_pose", 1000);	
	ros::Subscriber twist_sub= n.subscribe("/twist", 10, twist_callback); 
	
	srand((unsigned int)time(NULL));

	ros::Rate loop_rate(20); 
	while (ros::ok()){
		robot_tick(input_twist,0.05);
		robot_pose_publish(position);
		ros::spinOnce();
		loop_rate.sleep();
	} 
 	return 0;
}

