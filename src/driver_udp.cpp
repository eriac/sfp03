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

//for UDP
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 256


#include <vector>
#include <string>
#include <sstream>      // std::ostringstream
std::vector<std::string> split(const std::string &str, char sep){
    std::vector<std::string> v;
    std::stringstream ss(str);
    std::string buffer;
    while( std::getline(ss, buffer, sep) ) {
        v.push_back(buffer);
    }
    return v;
}
void set_ctrl(float *pos, float *dir){
	static tf::TransformBroadcaster br;
	tf::Transform transform;
	transform.setOrigin(  tf::Vector3(   pos[0], pos[1], pos[2]) );
	transform.setRotation(tf::Quaternion(dir[0], dir[1], dir[2], dir[3]));
	br.sendTransform(tf::StampedTransform(transform, ros::Time::now(), "world", "ctrl_link"));	
}
void set_sense(float *pos, float *dir){
	static tf::TransformBroadcaster br;
	tf::Transform transform;
	transform.setOrigin(  tf::Vector3(   pos[0], pos[1], pos[2]) );
	transform.setRotation(tf::Quaternion(dir[0], dir[1], dir[2], dir[3]));
	
	tf::Transform transform2;
	transform2.setOrigin(  tf::Vector3(   -0.03, 0.04, -0.19));
	transform2.setRotation(tf::Quaternion(dir[0], dir[1], dir[2], dir[3]));
	
	br.sendTransform(tf::StampedTransform(transform*transform2, ros::Time::now(), "world", "base_link"));	
}


int main(int argc, char **argv)
{
	ros::init(argc, argv, "test1");
	ros::NodeHandle n;

	/* ポート番号、ソケット */
	unsigned short port = 4001;
	int recvSocket;


	/* sockaddr_in 構造体 */
	struct sockaddr_in recvSockAddr;


	/* 各種パラメータ */
	int status;
	int numrcv;
	char buffer[BUFFER_SIZE];
	unsigned long on = 1;


	/* sockaddr_in 構造体のセット */
	memset(&recvSockAddr, 0, sizeof(recvSockAddr));
	recvSockAddr.sin_port = htons(port);
	recvSockAddr.sin_family = AF_INET;
	recvSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);


	/* ソケット生成 */
	recvSocket = socket(AF_INET, SOCK_DGRAM, 0);

	/* バインド */
	status = bind(recvSocket, (const struct sockaddr *) &recvSockAddr, sizeof(recvSockAddr));

	//select
	fd_set fds, readfds;
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	/* fd_setの初期化します */
	FD_ZERO(&readfds);
	/* selectで待つ読み込みソケットとしてsock1を登録します */
	FD_SET(recvSocket, &readfds);
	
	while (ros::ok()){
		memcpy(&fds, &readfds, sizeof(fd_set));
		int n = select(recvSocket+1, &fds, NULL, NULL, &tv);
		if(n>0){
			if (FD_ISSET(recvSocket, &fds)) {
				memset(buffer, 0, BUFFER_SIZE);
				numrcv = recvfrom(recvSocket, buffer, BUFFER_SIZE, 0, NULL, NULL);
				if(numrcv == -1) { status = close(recvSocket); break; }
				printf("received: %s\n", buffer);
				std::string recv_data=std::string(buffer);
				std::vector<std::string>process1=split(recv_data,',');
				
				//process
				if(process1.size()==9){
					std::string name=process1[0];
					std::string type=process1[1];
					float linear[3]={0};
					float angular[4]={0};
					for(int i=0;i<3;i++)linear[i] =atof(process1[i+2].c_str());
					for(int i=0;i<4;i++)angular[i]=atof(process1[i+5].c_str());
					printf("name:%s, type:%s\n",name.c_str(),type.c_str());
					printf("px%f, py%f,pz%f\n",linear[0],linear[1],linear[2]);
					printf("rx%f, ry%f,rz%f,rw%f\n",angular[0],angular[1],angular[2],angular[3]);
					if(name=="POINTER")set_ctrl(linear,angular);
					else if(name=="SENSOR")set_sense(linear,angular);
				}
			}
		}
		//sense_link to base_link
	} 
 	return 0;
}

