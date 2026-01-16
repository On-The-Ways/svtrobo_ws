#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float64.hpp>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>
#include <cmath>
#include <stdexcept>
#include <map>


struct RpmState
{
    double left  = 0.0;
    double right = 0.0;
    int zero_cnt = 0;
};


class ZLAC8015DRpmNode : public rclcpp::Node
{
public:
    ZLAC8015DRpmNode(const std::string& can_iface)
        : Node("zlac8015d_rpm_node"), can_iface_(can_iface)
    {
        openCanSocket();
        initPublishers();
        initDrivers();
    }

    ~ZLAC8015DRpmNode()
    {
        if (can_sock_ >= 0)
            close(can_sock_);
    }

    void spin()
    {
        rclcpp::Rate rate(10);  // 输出频率

        while (rclcpp::ok())
        {
            readCanFrame();
            publishAll();
            rclcpp::spin_some(this->get_node_base_interface());
            rate.sleep();
        }
    }

private:
    // ===== 参数 =====
    static constexpr int ZERO_CONFIRM_COUNT = 3;
    static constexpr double ZERO_THRESHOLD_RPM = 1.0;

    // ===== ROS =====
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr pub_front_left_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr pub_front_right_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr pub_rear_left_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr pub_rear_right_;

    // ===== CAN =====
    std::string can_iface_;
    int can_sock_;

    // ===== 驱动器状态 =====
    std::map<uint32_t, RpmState> driver_states_;
    uint32_t front_tpdo_id_;
    uint32_t rear_tpdo_id_;

    void initDrivers()
    {
        front_tpdo_id_ = 0x580 + 1; // node_id = 1
        rear_tpdo_id_  = 0x580 + 2; // node_id = 2

        driver_states_[front_tpdo_id_] = RpmState();
        driver_states_[rear_tpdo_id_]  = RpmState();
    }

    void initPublishers()
    {
        pub_front_left_  = this->create_publisher<std_msgs::msg::Float64>("front_left_rpm", 10);
        pub_front_right_ = this->create_publisher<std_msgs::msg::Float64>("front_right_rpm", 10);
        pub_rear_left_   = this->create_publisher<std_msgs::msg::Float64>("rear_left_rpm", 10);
        pub_rear_right_  = this->create_publisher<std_msgs::msg::Float64>("rear_right_rpm", 10);
    }

    void openCanSocket()
    {
        can_sock_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (can_sock_ < 0)
            throw std::runtime_error("Failed to create CAN socket");

        struct ifreq ifr;
        std::strncpy(ifr.ifr_name, can_iface_.c_str(), IFNAMSIZ);
        if (ioctl(can_sock_, SIOCGIFINDEX, &ifr) < 0)
            throw std::runtime_error("Failed to get CAN interface index");

        struct sockaddr_can addr {};
        addr.can_family  = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        if (bind(can_sock_, (struct sockaddr*)&addr, sizeof(addr)) < 0)
            throw std::runtime_error("Failed to bind CAN socket");

        // 同时接收两个 TPDO
        struct can_filter filters[2];
        filters[0].can_id   = 0x580 + 1;
        filters[0].can_mask = CAN_SFF_MASK;
        filters[1].can_id   = 0x580 + 2;
        filters[1].can_mask = CAN_SFF_MASK;

        setsockopt(can_sock_, SOL_CAN_RAW, CAN_RAW_FILTER,
                   &filters, sizeof(filters));
    }

    void readCanFrame()
    {
        struct can_frame frame {};
        int nbytes = read(can_sock_, &frame, sizeof(frame));
        if (nbytes <= 0)
            return;

        auto it = driver_states_.find(frame.can_id);
        if (it == driver_states_.end())
            return;

        if (frame.can_dlc != 8)
            return;

        const uint8_t* d = frame.data;

        // 0x606C : 03
        if (d[1] == 0x6C && d[2] == 0x60 && d[3] == 0x03)
        {
            int16_t left_raw  = static_cast<int16_t>(d[4] | (d[5] << 8));
            int16_t right_raw = static_cast<int16_t>(d[6] | (d[7] << 8));

            double left_rpm  = left_raw  / 10.0;
            double right_rpm = right_raw / 10.0;

            RpmState& st = it->second;

            if (std::abs(left_rpm) < ZERO_THRESHOLD_RPM &&
                std::abs(right_rpm) < ZERO_THRESHOLD_RPM)
            {
                st.zero_cnt++;
                if (st.zero_cnt >= ZERO_CONFIRM_COUNT)
                {
                    st.left  = 0.0;
                    st.right = 0.0;
                }
            }
            else
            {
                st.zero_cnt = 0;
                st.left  = left_rpm;
                st.right = right_rpm;
            }
        }
    }

    void publishAll()
    {
        publishPair(pub_front_left_, pub_front_right_,
                    driver_states_[front_tpdo_id_]);

        publishPair(pub_rear_left_, pub_rear_right_,
                    driver_states_[rear_tpdo_id_]);
    }

    static void publishPair(rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr& left_pub,
                            rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr& right_pub,
                            const RpmState& st)
    {
        std_msgs::msg::Float64 msg;

        msg.data = st.left;
        left_pub->publish(msg);

        msg.data = st.right;
        right_pub->publish(msg);
    }
};


int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);

    std::string can_iface = "can2";
    
    auto node = std::make_shared<ZLAC8015DRpmNode>(can_iface);

    try
    {
        rclcpp::spin(node);
    }
    catch (const std::exception& e)
    {
        RCLCPP_ERROR(node->get_logger(), "Exception: %s", e.what());
        rclcpp::shutdown();
        return 1;
    }

    rclcpp::shutdown();
    return 0;
}
