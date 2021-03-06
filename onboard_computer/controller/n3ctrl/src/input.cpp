#include "input.h"

using uav_utils::in_range;
//在这加个全局变量用来计时
RC_Data_t::RC_Data_t() {
    rcv_stamp = ros::Time(0);

    last_mode = -1.0;
    set_default_mode(std::string("manual"));
    set_default_mode(std::string("noapi"));

    is_command_mode = false;
    enter_command_mode = false;
    is_api_mode = false;
    enter_api_mode = false;
}

void RC_Data_t::set_default_mode(std::string s) {
    if (s == "manual") {
        last_gear = MANUAL_MODE_GEAR_VALUE;//这里不用管，gear不起作用
    } else if (s == "command") {
        last_gear = COMMAND_MODE_GEAR_VALUE;
    } else if (s == "api") {
        last_mode = 0.8;
    } else if (s == "noapi") {
        last_mode = -0.8;
    } else {
        ROS_ASSERT_MSG(false, "Invalid mode for RC");
    }
}

void RC_Data_t::feed(sensor_msgs::JoyConstPtr pMsg) {// author:wkd  modified:2021-5
    msg = *pMsg;
    rcv_stamp = ros::Time::now();

    roll = msg.axes[0];
    pitch = msg.axes[1];
    yaw = msg.axes[2];
    thr = msg.axes[3];
    mode = msg.axes[4];
    gear = msg.axes[5];
    //if(gear < -1.0) gear = -1.0; //zxzxzxzx
    //这个gear的判断不要了
    check_validity(); //这里面gear的取值范围要改

    if (last_mode < API_MODE_THRESHOLD_VALUE && mode > API_MODE_THRESHOLD_VALUE)
    {
        enter_api_mode = true; //在这加个开始计时的点
        enter_command_mode = true;// wkd 2021-5
    }
        
    else
    {
        enter_api_mode = false;
        //enter_command_mode = false; //这个地方应该是个比较关键的地方，与enter api mode不同，enter command mode一旦为true便一直为true
    }
        

    if (mode > API_MODE_THRESHOLD_VALUE)
    {
        is_api_mode = true;
        is_command_mode = true;// wkd 2021-5
    }
        
    else
    {
        is_api_mode = false;
        is_command_mode = false;// wkd 2021-5
    }
        
/*  去掉gear物理开关的部分，gear和mode合并
    if (last_gear < GEAR_SHIFT_VALUE && gear > GEAR_SHIFT_VALUE) {
        enter_command_mode = true;
    } else if (gear < GEAR_SHIFT_VALUE) {
        enter_command_mode = false;
    }
    //在这进行判断，3秒之后进is command mode 5秒之后进enter command mode
    if (gear > GEAR_SHIFT_VALUE)
        is_command_mode = true;
    else
        is_command_mode = false;
*/
    last_mode = mode;
    last_gear = gear;
}

bool RC_Data_t::check_enter_command_mode() {
    if (enter_command_mode) {
        enter_command_mode = false;
        return true;
    } else {
        return false;
    }
}

void RC_Data_t::check_validity() {//这里面把gear的判断去掉，因为gear不作为物理开关使用
    if (in_range(roll, 1.0) && in_range(pitch, 1.0) && in_range(yaw, 1.0) && in_range(thr, 1.0) &&
        in_range(mode, -1.0, 1.0)) {// author:wkd  modified:2021-5
        // pass
    } else {
        ROS_ERROR("RC data validity check fail.");
    }
}

Odom_Data_t::Odom_Data_t() {
    rcv_stamp = ros::Time(0);
    q.setIdentity();
};

void Odom_Data_t::feed(nav_msgs::OdometryConstPtr pMsg) {
    msg = *pMsg;
    rcv_stamp = ros::Time::now();

    uav_utils::extract_odometry(pMsg, p, v, q, w);
}

Imu_Data_t::Imu_Data_t() {
    rcv_stamp = ros::Time(0);
}

void Imu_Data_t::feed(sensor_msgs::ImuConstPtr pMsg) {
    msg = *pMsg;
    rcv_stamp = ros::Time::now();

    w(0) = msg.angular_velocity.x;
    w(1) = msg.angular_velocity.y;
    w(2) = msg.angular_velocity.z;

    a(0) = msg.linear_acceleration.x;
    a(1) = msg.linear_acceleration.y;
    a(2) = msg.linear_acceleration.z;

    q.x() = msg.orientation.x;
    q.y() = msg.orientation.y;
    q.z() = msg.orientation.z;
    q.w() = msg.orientation.w;
}

Command_Data_t::Command_Data_t() {
    rcv_stamp = ros::Time(0);
    trajectory_id = 0;
    trajectory_flag = 0;
}

void Command_Data_t::feed(quadrotor_msgs::PositionCommandConstPtr pMsg) {

    // if(msg.trajectory_id == 0)
    // {
    //     //The first trajectory_id must be greater than 0. zxzxzxzx
    //     return;
    // }

    msg = *pMsg;
    rcv_stamp = ros::Time::now();

    p(0) = msg.position.x;
    p(1) = msg.position.y;
    p(2) = msg.position.z;

    v(0) = msg.velocity.x;
    v(1) = msg.velocity.y;
    v(2) = msg.velocity.z;

    a(0) = msg.acceleration.x;
    a(1) = msg.acceleration.y;
    a(2) = msg.acceleration.z;

    yaw = uav_utils::normalize_angle(msg.yaw);

    trajectory_id = msg.trajectory_id;
    trajectory_flag = msg.trajectory_flag;
}

Idling_Data_t::Idling_Data_t() {
    rcv_stamp = ros::Time(0);
    need_idling = false;
}

void Idling_Data_t::feed(geometry_msgs::Vector3StampedConstPtr pMsg) {
    msg = *pMsg;
    rcv_stamp = ros::Time::now();

    need_idling = (msg.header.frame_id.compare(std::string("idling")) == 0);
}

Trigger_Data_t::Trigger_Data_t() : need_enter_js(false){};

void Trigger_Data_t::feed(std_msgs::HeaderConstPtr pMsg) {
    need_enter_js = true;
}

bool Trigger_Data_t::get_enter_js() {
    if (need_enter_js) {
        need_enter_js = false;
        return true;
    } else {
        return false;
    }
}
