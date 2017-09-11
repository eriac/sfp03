#include "ros/ros.h"
  
#include "math.h"
#include "std_msgs/Float32MultiArray.h"
#include "geometry_msgs/PoseStamped.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/JointState.h"
#include "sensor_msgs/Joy.h"
#include <tf/transform_broadcaster.h>

#include <string>
#include <iostream>
#include <sstream>

ros::Publisher goal_pub;
float input[6]={0};

float position[3]={0.0,0.0,1.0};
float direction[3]={0.0,0.0};		

void twist_callback(const geometry_msgs::Twist& twist_msg){
	input[0]=twist_msg.linear.x;
	input[1]=twist_msg.linear.y;
	input[2]=twist_msg.angular.y;
	input[3]=twist_msg.angular.z;
	input[4]=twist_msg.linear.z;
	input[5]=twist_msg.angular.x;
}
void set_ctrl(float *position, float *direction){
	static tf::TransformBroadcaster br;
	tf::Transform transform;
	transform.setOrigin( tf::Vector3(position[0], position[1], position[2]) );
	tf::Quaternion q;
	q.setRPY(direction[2], direction[1], direction[0]);
	transform.setRotation(q);
	br.sendTransform(tf::StampedTransform(transform, ros::Time::now(), "world", "ctrl_link"));	
}
int main(int argc, char **argv)
{
	ros::init(argc, argv, "test1");
	ros::NodeHandle n;
	
	ros::Subscriber twist_sub   = n.subscribe("/ctrl_twist", 10, twist_callback); 

	ros::Rate loop_rate(20); 
	while (ros::ok()){
		position[0] +=input[0]*0.05;
		position[1] +=input[1]*0.05;
		direction[0]+=input[2]*0.05;
		direction[1]+=input[3]*0.05;
		position[2] +=input[4]*0.05;
		direction[2]+=input[5]*0.05;

		set_ctrl(position,direction);

		ros::spinOnce();
		loop_rate.sleep();
	} 
 	return 0;
}

