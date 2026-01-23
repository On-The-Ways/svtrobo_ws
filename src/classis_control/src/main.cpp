#include <atomic>
#include <iostream>
#include <memory>
#include <rclcpp/node.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/float64.hpp>
#include <std_msgs/msg/int32.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <thread>
#include <unistd.h>
#include <vector>
#include <mutex>
#include <exception>
#include <chrono>
#include <cmath>

#include "stdint.h"

#include "classis_control/steering_motor.h"
#include "classis_control/wheel_motor.h"

#define STEER_MOTOR_cAN "can2" //
#define WHEEL_MOTOR_CAN "can3"

#define PI 3.1415926535897932384626433832795f

#define CHASSIS_RADIUS         0.2121f       //底盘半径单位m
#define WHEEL_PERIMETER         0.647f       //轮子周长

#define FRONT_LEFT_START_ANGLE 4.8f
#define FRONT_RIGHT_START_ANGLE 5.0f
#define REAR_LEFT_START_ANGLE 6.1f
#define REAR_RIGHT_START_ANGLE 4.5f

#define motor_kp 4.0f
#define motor_kd 0.4f

// 滤波参数
#define FILTER_ALPHA 0.15f        // 低通滤波系数 (0-1, 越小越平滑但响应越慢)
#define MAX_ANGLE_RATE 2.0f       // 最大角度变化率 (rad/s)
#define MAX_WHEEL_SPEED 100.0f    // 最大轮子速度 (rpm)，根据实际电机能力调整

#define WHEEL_FL_DIRETION 1
#define WHEEL_FR_DIRETION -1
#define WHEEL_RL_DIRETION 1
#define WHEEL_RR_DIRETION -1

std::atomic<double> chassis_vx_set{0.0};        //m/s
std::atomic<double> chassis_vy_set{0.0};
std::atomic<double> chassis_wz_set{0.0};

typedef struct chassis_control_para_t
{
    double vx_set;                  //m/s
    double vy_set;                  //m/s
    double wz_set;                  //rad/s
    double front_left_angle;
    double front_right_angle;
    double rear_left_angle;
    double rear_right_angle;
    double front_left_speed;        //rad/s
    double front_right_speed;
    double rear_left_speed;
    double rear_right_speed;
    /* data */
} chassis_control_para_t;
chassis_control_para_t chassis_control_para;

static void chassis_init(void)
{
    chassis_control_para.front_left_angle = FRONT_LEFT_START_ANGLE;
    chassis_control_para.front_right_angle = FRONT_RIGHT_START_ANGLE;
    chassis_control_para.rear_left_angle = REAR_LEFT_START_ANGLE;
    chassis_control_para.rear_right_angle = REAR_RIGHT_START_ANGLE;
}

static void chassis_set_control(double vx, double vy, double wz)
{
    chassis_control_para.vx_set = vx;
    chassis_control_para.vy_set = vy;
    chassis_control_para.wz_set = wz;
}

static void chassis_control_loop(double vx_set, double vy_set, double wz_set)
{
    //航向控制
    float wheel_rpm_ratio;
	
    wheel_rpm_ratio = 60.0f/WHEEL_PERIMETER;	
	
    chassis_control_para.front_left_speed = sqrt(	pow(vy_set + wz_set * CHASSIS_RADIUS * 0.707107f,2)
                       +	pow(vx_set - wz_set * CHASSIS_RADIUS * 0.707107f,2)
                       ) * wheel_rpm_ratio ;
    chassis_control_para.rear_left_speed = sqrt(	pow(vy_set - wz_set * CHASSIS_RADIUS * 0.707107f,2)
                       +	pow(vx_set - wz_set * CHASSIS_RADIUS * 0.707107f,2)
                       ) * wheel_rpm_ratio ;
    chassis_control_para.front_right_speed = sqrt(	pow(vy_set + wz_set * CHASSIS_RADIUS * 0.707107f,2)
                       +	pow(vx_set + wz_set * CHASSIS_RADIUS * 0.707107f,2)
                       ) * wheel_rpm_ratio ;
    chassis_control_para.rear_right_speed = sqrt(	pow(vy_set + wz_set * CHASSIS_RADIUS * 0.707107f,2)
                       +	pow(vx_set - wz_set * CHASSIS_RADIUS * 0.707107f,2) 
                       ) * wheel_rpm_ratio ;
    
    // // 按比例限制速度：找出最大速度，如果超过限制则按比例缩放所有速度
    double max_speed = fabs(chassis_control_para.front_left_speed);
    if (fabs(chassis_control_para.front_right_speed) > max_speed)
        max_speed = fabs(chassis_control_para.front_right_speed);
    if (fabs(chassis_control_para.rear_left_speed) > max_speed)
        max_speed = fabs(chassis_control_para.rear_left_speed);
    if (fabs(chassis_control_para.rear_right_speed) > max_speed)
        max_speed = fabs(chassis_control_para.rear_right_speed);
    
    // // 如果最大速度超过限制，按比例缩放所有速度（保持运动方向）
    if (max_speed > MAX_WHEEL_SPEED) {
        double scale = MAX_WHEEL_SPEED / max_speed;
        chassis_control_para.front_left_speed *= scale;
        chassis_control_para.front_right_speed *= scale;
        chassis_control_para.rear_left_speed *= scale;
        chassis_control_para.rear_right_speed *= scale;
    }

    double fl_angle = atan2((vy_set + wz_set * CHASSIS_RADIUS * 0.707107f),(vx_set - wz_set * CHASSIS_RADIUS * 0.707107f));
    double fr_angle = atan2((vy_set + wz_set * CHASSIS_RADIUS * 0.707107f),(vx_set + wz_set * CHASSIS_RADIUS * 0.707107f));
    double rl_angle = atan2((vy_set - wz_set * CHASSIS_RADIUS * 0.707107f),(vx_set - wz_set * CHASSIS_RADIUS * 0.707107f));
    double rr_angle = atan2((vy_set - wz_set * CHASSIS_RADIUS * 0.707107f),(vx_set + wz_set * CHASSIS_RADIUS * 0.707107f));

    if(fabs(fl_angle)>PI/2.0f)
    {
      if(fl_angle>0.0f)
      {
        fl_angle = -(PI - fabs(fl_angle));
        chassis_control_para.front_left_speed = -chassis_control_para.front_left_speed;
      }
      else
      {
        fl_angle = PI - fabs(fl_angle);
        chassis_control_para.front_left_speed = -chassis_control_para.front_left_speed;
      }
    }

    if(fabs(fr_angle)>PI/2.0f)
    {
      if(fr_angle>0.0f)
      {
        fr_angle = -(PI - fabs(fr_angle));
        chassis_control_para.front_right_speed = -chassis_control_para.front_right_speed;
      }
      else
      {
        fr_angle = PI - fabs(fr_angle);
        chassis_control_para.front_right_speed = -chassis_control_para.front_right_speed;
      }
    }

    if(fabs(rl_angle)>PI/2.0f)
    {
      if(rl_angle>0.0f)
      {
        rl_angle = -(PI - fabs(rl_angle));
        chassis_control_para.rear_left_speed = -chassis_control_para.rear_left_speed;
      }
      else
      {
        fl_angle = PI - fabs(fl_angle);
        chassis_control_para.rear_left_speed = -chassis_control_para.rear_left_speed;
      }
    }

    if(fabs(rr_angle)>PI/2.0f)
    {
      if(rr_angle>0.0f)
      {
        rr_angle = -(PI - fabs(rr_angle));
        chassis_control_para.rear_right_speed = -chassis_control_para.rear_right_speed;
      }
      else
      {
        rr_angle = PI - fabs(rr_angle);
        chassis_control_para.rear_right_speed = -chassis_control_para.rear_right_speed;
      }
    }
    //舵向控制
    chassis_control_para.front_left_angle = fl_angle + FRONT_LEFT_START_ANGLE;        
    chassis_control_para.front_right_angle = fr_angle + FRONT_RIGHT_START_ANGLE;
    chassis_control_para.rear_left_angle = rl_angle  + REAR_LEFT_START_ANGLE;
    chassis_control_para.rear_right_angle = rr_angle + REAR_RIGHT_START_ANGLE;


}

// 低通滤波器类
class LowPassFilter {
public:
  LowPassFilter(double alpha = FILTER_ALPHA) : alpha_(alpha), output_(0.0), initialized_(false) {}
  
  double filter(double input) {
    if (!initialized_) {
      output_ = input;
      initialized_ = true;
      return output_;
    }
    // 一阶低通滤波: y[n] = alpha * x[n] + (1-alpha) * y[n-1]
    output_ = alpha_ * input + (1.0 - alpha_) * output_;
    return output_;
  }
  
  void reset() {
    initialized_ = false;
    output_ = 0.0;
  }
  
  double getOutput() const { return output_; }
  
private:
  double alpha_;
  double output_;
  bool initialized_;
};

// 角度变化率限制器
class RateLimiter {
public:
  RateLimiter(double max_rate = MAX_ANGLE_RATE) : max_rate_(max_rate), last_output_(0.0), initialized_(false) {}
  
  double limit(double target, double dt) {
    if (!initialized_) {
      last_output_ = target;
      initialized_ = true;
      return target;
    }
    
    double diff = target - last_output_;
    double max_change = max_rate_ * dt;
    
    if (diff > max_change) {
      last_output_ += max_change;
    } else if (diff < -max_change) {
      last_output_ -= max_change;
    } else {
      last_output_ = target;
    }
    
    return last_output_;
  }
  
  void reset() {
    initialized_ = false;
    last_output_ = 0.0;
  }
  
private:
  double max_rate_;
  double last_output_;
  bool initialized_;
};

class classis_control : public rclcpp::Node {
public:
  classis_control()
      : rclcpp::Node("motor_control_set_node"),
        motor1(RobStrideMotor(STEER_MOTOR_cAN, 0xFF, 0x65, 0)),
        motor2(RobStrideMotor(STEER_MOTOR_cAN, 0xFF, 0x66, 0)), 
        motor3(RobStrideMotor(STEER_MOTOR_cAN, 0xFF, 0x67, 0)),
        motor4(RobStrideMotor(STEER_MOTOR_cAN, 0xFF, 0x68, 0)),
        fl_angle_filter(FILTER_ALPHA),
        fr_angle_filter(FILTER_ALPHA),
        rl_angle_filter(FILTER_ALPHA),
        rr_angle_filter(FILTER_ALPHA),
        fl_rate_limiter(MAX_ANGLE_RATE),
        fr_rate_limiter(MAX_ANGLE_RATE),
        rl_rate_limiter(MAX_ANGLE_RATE),
        rr_rate_limiter(MAX_ANGLE_RATE){
    svtrobot_cmd_sub = this->create_subscription<geometry_msgs::msg::Twist>("/svtrobot_cmd", 10, std::bind(&classis_control::svtrobot_cmd_callback, this, std::placeholders::_1));

    // pub_front_left = this->create_publisher<std_msgs::msg::Int32>("/front_left_cmd", 10);
    // pub_front_right = this->create_publisher<std_msgs::msg::Int32>("/front_right_cmd", 10);
    // pub_rear_left = this->create_publisher<std_msgs::msg::Int32>("/rear_left_cmd", 10);
    // pub_rear_right = this->create_publisher<std_msgs::msg::Int32>("/rear_right_cmd", 10);

    motor1.Get_RobStrite_Motor_parameter(0x7005);
    usleep(100);
    motor2.Get_RobStrite_Motor_parameter(0x7005);
    usleep(100);
    motor3.Get_RobStrite_Motor_parameter(0x7005);
    usleep(100);
    motor4.Get_RobStrite_Motor_parameter(0x7005);
    usleep(100);

    motor1.enable_motor();
    usleep(100);
    motor2.enable_motor();
    usleep(100);
    motor3.enable_motor();
    usleep(100);
    motor4.enable_motor();
    usleep(100);

    RCLCPP_INFO(this->get_logger(), "Creating front and rear ZLAC8015D on interface '%s'...", WHEEL_MOTOR_CAN);
    try 
    {
      front_ = std::make_unique<ZLAC8015D>(WHEEL_MOTOR_CAN, 1, 0.3);
      rear_  = std::make_unique<ZLAC8015D>(WHEEL_MOTOR_CAN, 2, 0.3);

      for (auto *drv : std::vector<ZLAC8015D*>{front_.get(), rear_.get()}) 
      {
        int hb = drv->wait_heartbeat(1.0);
        (void)hb;
        try 
        { 
          drv->clear_fault(); 
          } 
        catch (...) 
        {

        }
        drv->set_velocity_mode();
        drv->enable_operation();
      }
    } 
    catch (const std::exception &e) 
    {
      RCLCPP_ERROR(this->get_logger(), "Error during initial device setup: %s", e.what());
      throw;
    }
    signal(SIGINT, [](int sig) 
    {
      (void)sig;
      rclcpp::shutdown();
    });
    RCLCPP_INFO(this->get_logger(), "chassis is initing");
    chassis_init();

    


    RCLCPP_INFO(this->get_logger(), "chassis init finished");
    worker_thread_ = std::thread(&classis_control::excute_loop, this);
  }

  ~classis_control () {

    motor1.Disenable_Motor(0);
    motor2.Disenable_Motor(0);
    motor3.Disenable_Motor(0);
    motor4.Disenable_Motor(0);


    running_ = false; // 停止线程
    if (worker_thread_.joinable())
      worker_thread_.join(); // 等待线程结束
  }

  void excute_loop() {
    const double LOOP_DT = 0.001;      // 循环周期 1ms
    
    // 记录上次循环时间
    auto last_time = std::chrono::steady_clock::now();
    
    while (running_) {
      try {
        // 计算实际循环时间
        auto current_time = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(current_time - last_time).count();
        last_time = current_time;
        
        // 限制dt在合理范围内，避免异常值
        if (dt > 0.1) dt = LOOP_DT;  // 如果dt过大，使用默认值
        if (dt < 0.0001) dt = LOOP_DT;  // 如果dt过小，使用默认值
        
        std_msgs::msg::Int32 fl_msg, fr_msg, rl_msg, rr_msg;
        
        // 使用原子变量读取速度指令（线程安全）
        double vx = chassis_vx_set;
        double vy = chassis_vy_set;
        double wz = chassis_wz_set;
        
        chassis_set_control(vx, vy, wz);
        chassis_control_loop(chassis_control_para.vx_set, chassis_control_para.vy_set, chassis_control_para.wz_set);
        
        RCLCPP_INFO(this->get_logger(), "motor1 angle: %f",chassis_control_para.front_left_angle);
        RCLCPP_INFO(this->get_logger(), "motor2 angle: %f",chassis_control_para.front_right_angle);
        RCLCPP_INFO(this->get_logger(), "motor3 angle: %f",chassis_control_para.rear_right_angle);
        RCLCPP_INFO(this->get_logger(), "motor4 angle: %f",chassis_control_para.rear_left_angle);

        double fl_angle_target = chassis_control_para.front_left_angle;
    double fr_angle_target = chassis_control_para.front_right_angle;
    double rl_angle_target = chassis_control_para.rear_left_angle;
    double rr_angle_target = chassis_control_para.rear_right_angle;
    
            // 应用变化率限制
    double fl_angle_limited = fl_rate_limiter.limit(fl_angle_target, dt);
    double fr_angle_limited = fr_rate_limiter.limit(fr_angle_target, dt);
    double rl_angle_limited = rl_rate_limiter.limit(rl_angle_target, dt);
    double rr_angle_limited = rr_rate_limiter.limit(rr_angle_target, dt);
        
        // 应用低通滤波
    double fl_angle = fl_angle_filter.filter(fl_angle_limited);
    double fr_angle = fr_angle_filter.filter(fr_angle_limited);
    double rl_angle = rl_angle_filter.filter(rl_angle_limited);
    double rr_angle = rr_angle_filter.filter(rr_angle_limited);

// 验证角度值有效性（检查NaN和Inf）
    if (!std::isfinite(fl_angle)) {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                           "Invalid fl_angle: %f, using previous value", fl_angle);
      fl_angle = fl_angle_filter.getOutput();  // 使用上次有效值
    }
    if (!std::isfinite(fr_angle)) {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000,   
                           "Invalid fr_angle: %f, using previous value", fr_angle);
      fr_angle = fr_angle_filter.getOutput();
    }  
    if (!std::isfinite(rl_angle)) {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                           "Invalid rl_angle: %f, using previous value", rl_angle);
      rl_angle = rl_angle_filter.getOutput();
    }
    if (!std::isfinite(rr_angle)) {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                           "Invalid rr_angle: %f, using previous value", rr_angle);
      rr_angle = rr_angle_filter.getOutput();
    }
     try {
          float fl_angle_f = static_cast<float>(fl_angle);
          if (std::isfinite(fl_angle_f)) {
            motor1.send_motion_command(0.0f, fl_angle_f, 0.0f, motor_kp, motor_kd);
          } else {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                                 "Motor1: Skipping invalid angle %f", fl_angle_f);
          }
        } catch (const std::exception& e) {
          RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                               "Motor1 command failed: %s", e.what());
        }
        
        try {
          float fr_angle_f = static_cast<float>(fr_angle);
          if (std::isfinite(fr_angle_f)) {
            motor2.send_motion_command(0.0f, fr_angle_f, 0.0f, motor_kp, motor_kd);
          } else {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                                 "Motor2: Skipping invalid angle %f", fr_angle_f);
          }
        } catch (const std::exception& e) {
          RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                               "Motor2 command failed: %s", e.what());
        }
        
        try {
          float rl_angle_f = static_cast<float>(rl_angle);
          if (std::isfinite(rl_angle_f)) {
            motor3.send_motion_command(0.0f, rl_angle_f, 0.0f, motor_kp, motor_kd);
          } else {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                                 "Motor3: Skipping invalid angle %f", rl_angle_f);
          }
        } catch (const std::exception& e) {
          RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                               "Motor3 command failed: %s", e.what());
        }
        
        try {
          float rr_angle_f = static_cast<float>(rr_angle);
          if (std::isfinite(rr_angle_f)) {
            motor4.send_motion_command(0.0f, rr_angle_f, 0.0f, motor_kp, motor_kd);
          } else {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                                 "Motor4: Skipping invalid angle %f", rr_angle_f);
          }
        } catch (const std::exception& e) {
          RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                               "Motor4 command failed: %s", e.what());
        }
        // 发送电机控制命令，添加异常处理和数据验证
        
      }catch (const std::exception& e) {
        RCLCPP_ERROR_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                              "Error in control loop: %s", e.what());
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 出错时稍长等待
      }
      if((fabs(chassis_control_para.rear_right_angle-motor4.position_)<=0.1)
        && fabs(chassis_control_para.rear_left_angle-motor3.position_)<=0.1
        && fabs(chassis_control_para.front_right_angle-motor2.position_)<=0.1
        && fabs(chassis_control_para.front_left_angle-motor1.position_)<=0.1)
      {
          front_->set_target_speed_lr_rpm(chassis_control_para.front_left_speed*WHEEL_FL_DIRETION,
                                        chassis_control_para.front_right_speed*WHEEL_FR_DIRETION);
          rear_->set_target_speed_lr_rpm(chassis_control_para.rear_right_speed*WHEEL_RR_DIRETION,
                                        chassis_control_para.rear_left_speed*WHEEL_RL_DIRETION);
  
      }
      else
      {
        front_->set_target_speed_lr_rpm(0,0);
        rear_->set_target_speed_lr_rpm(0,0);
      }
    }

  } 

private:
  std::thread worker_thread_;
  std::atomic<bool> running_ = true;

  RobStrideMotor motor1;
  RobStrideMotor motor2;
  RobStrideMotor motor3;
  RobStrideMotor motor4;
  
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr svtrobot_cmd_sub;
  // 角度滤波器（每个电机一个）
  LowPassFilter fl_angle_filter;
  LowPassFilter fr_angle_filter;
  LowPassFilter rl_angle_filter;
  LowPassFilter rr_angle_filter;
  
  // 角度变化率限制器（每个电机一个）
  RateLimiter fl_rate_limiter;
  RateLimiter fr_rate_limiter;
  RateLimiter rl_rate_limiter;
  RateLimiter rr_rate_limiter;

  double control_rate_hz_;

  std::unique_ptr<ZLAC8015D> front_;
  std::unique_ptr<ZLAC8015D> rear_;

  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr pub_front_left;
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr pub_front_right;
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr pub_rear_left;
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr pub_rear_right;
  void svtrobot_cmd_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
  {
    chassis_vx_set = msg->linear.x;
    chassis_vy_set = msg->linear.y;
    chassis_wz_set = msg->angular.z;
  }

};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  auto controller = std::make_shared<classis_control>();

  rclcpp::executors::MultiThreadedExecutor executor;

  executor.add_node(controller);

  executor.spin();

  rclcpp::shutdown();

  return 0;
}