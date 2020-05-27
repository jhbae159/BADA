#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

void moveToHere(float x, float y, float orien_z, float orien_w){


  //tell the action client that we want to spin a thread by default
  MoveBaseClient ac("move_base", true);

  //wait for the action server to come up
  ROS_INFO("Waiting for the move_base action server to come up");
  ac.waitForServer();
  move_base_msgs::MoveBaseGoal goal;

  //we'll send a goal to the robot to move 1 meter forward
  goal.target_pose.header.frame_id = "map";
  goal.target_pose.header.stamp = ros::Time::now();

  goal.target_pose.pose.position.x = x;
  goal.target_pose.pose.position.x = y;
  goal.target_pose.pose.orientation.z = orien_z;
  goal.target_pose.pose.orientation.w = orien_w;

  ROS_INFO("Sending goal");
  ac.sendGoal(goal);

  ac.waitForResult();

  if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
    ROS_INFO("bada moving success!");
  else
    ROS_INFO("bada moving fail!");

  return;
}

int main(int argc, char** argv){
  	ros::init(argc, argv, "navigation_goals");
	moveToHere(1,1,0,1);
	return 0;
}
