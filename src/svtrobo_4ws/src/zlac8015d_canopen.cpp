/******************************************************************************
 * @file    zlac8015d_canopen.cpp
 * @brief   ZLAC8015D CANopen SDO control implementation - ROS2 Humble
 *
 * @author  luzhongfa
 * @company 杭州时空变量科技有限公司
 * @date    2025-12-24
 *
 *****************************************************************************/

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/int32.hpp>

#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <ifaddrs.h>
#include <iostream>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <memory>
#include <mutex>
#include <net/if.h>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <poll.h>
#include <sstream>
#include <iomanip>
#include <optional>
#include <atomic>

using namespace std::chrono_literals;


class CanopenSdoError : public std::runtime_error {
public:
    explicit CanopenSdoError(const std::string &msg) : std::runtime_error(msg) {}
};

class SocketCan
{
public:
    // Create socket bound to ifname, with optional filters
    explicit SocketCan(const std::string &ifname, const std::vector<can_filter>& filters) {
        sock_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (sock_ < 0) {
            throw std::runtime_error("socket(PF_CAN) failed: " + std::string(strerror(errno)));
        }

        // bind to interface
        struct ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        // ensure null terminated copy
        std::strncpy(ifr.ifr_name, ifname.c_str(), IFNAMSIZ - 1);
        ifr.ifr_name[IFNAMSIZ - 1] = '\0';

        if (ioctl(sock_, SIOCGIFINDEX, &ifr) < 0) {
            close(sock_);
            throw std::runtime_error("SIOCGIFINDEX failed for " + ifname + ": " + std::string(strerror(errno)));
        }
        struct sockaddr_can addr {};
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        // disable receiving our own transmitted frames (avoid kernel loopback echo)
        int loopback = 0;
        if (setsockopt(sock_, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &loopback, sizeof(loopback)) < 0) {
            // non-fatal: warn
            std::ostringstream ss;
            ss << "Warning: CAN_RAW_RECV_OWN_MSGS setsockopt failed: " << strerror(errno);
            std::cerr << ss.str() << std::endl;
        }

        // optionally set error filter to ignore error frames
        int err_mask = 0;
        if (setsockopt(sock_, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask)) < 0) {
            // non-fatal: warn
            std::ostringstream ss;
            ss << "Warning: CAN_RAW_ERR_FILTER setsockopt failed: " << strerror(errno);
            std::cerr << ss.str() << std::endl;
        }

        // set filters if provided
        if (!filters.empty()) {
            if (setsockopt(sock_, SOL_CAN_RAW, CAN_RAW_FILTER, filters.data(), static_cast<int>(filters.size() * sizeof(can_filter))) < 0) {
                close(sock_);
                throw std::runtime_error("setsockopt CAN_RAW_FILTER failed: " + std::string(strerror(errno)));
            }
        }

        // bind
        if (bind(sock_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
            close(sock_);
            throw std::runtime_error("bind CAN socket failed: " + std::string(strerror(errno)));
        }
    }

    ~SocketCan() {
        if (sock_ >= 0) close(sock_);
    }

    // send a frame (thread-safe)
    void send_frame(const struct can_frame &frame) {
        std::lock_guard<std::mutex> lk(tx_mutex_);
        ssize_t n = write(sock_, &frame, sizeof(frame));
        if (n != static_cast<ssize_t>(sizeof(frame))) {
            std::ostringstream ss;
            ss << "CAN send failed: wrote=" << n << " (" << strerror(errno) << ")";
            throw std::runtime_error(ss.str());
        }
    }

    // receive with timeout (ms). If timeout_ms < 0 -> block indefinitely.
    // Returns pair<bool, can_frame>. bool=false => timeout/no frame.
    std::pair<bool, struct can_frame> recv_frame(int timeout_ms) {
        struct pollfd pfd;
        pfd.fd = sock_;
        pfd.events = POLLIN;
        int ret = poll(&pfd, 1, timeout_ms);
        if (ret <= 0) {
            return {false, {}};
        }
        struct can_frame frame;
        ssize_t n = read(sock_, &frame, sizeof(frame));
        if (n < 0) {
            std::ostringstream ss;
            ss << "CAN read failed: " << strerror(errno);
            throw std::runtime_error(ss.str());
        }
        if (n < static_cast<ssize_t>(sizeof(struct can_frame))) {
            std::ostringstream ss;
            ss << "Short CAN frame read: n=" << n;
            throw std::runtime_error(ss.str());
        }
        return {true, frame};
    }

    // Non-blocking drain (max iterations) using MSG_DONTWAIT recv
    void drain(int max_iters = 200) {
        for (int i = 0; i < max_iters; ++i) {
            struct can_frame f;
            ssize_t n = recv(sock_, &f, sizeof(f), MSG_DONTWAIT);
            if (n <= 0) break;
        }
    }

private:
    int sock_{-1};
    std::mutex tx_mutex_;
};

class ZLAC8015D {
public:
    // primary constructor: create its own socket (default)
    ZLAC8015D(const std::string &channel, int node_id, double recv_timeout = 0.3)
        : node_id_(node_id),
          sdo_tx_(0x600 + node_id),
          sdo_rx_(0x580 + node_id),
          hb_id_(0x700 + node_id),
          recv_timeout_s_(recv_timeout)
    {
        init_socket(channel);
    }

    // secondary constructor: accept externally created socket (optional)
    ZLAC8015D(std::shared_ptr<SocketCan> external_socket, int node_id, double recv_timeout = 0.3)
        : node_id_(node_id),
          sdo_tx_(0x600 + node_id),
          sdo_rx_(0x580 + node_id),
          hb_id_(0x700 + node_id),
          recv_timeout_s_(recv_timeout),
          socket_(std::move(external_socket))
    {
        if (!socket_) throw std::invalid_argument("external_socket is null");
    }

    ~ZLAC8015D() = default;

    // NMT start
    void nmt_start() {
        struct can_frame f {};
        f.can_id = 0x000;
        f.can_dlc = 2;
        f.data[0] = 0x01;
        f.data[1] = static_cast<uint8_t>(node_id_);
        socket_->send_frame(f);
    }

    // Wait heartbeat; returns optional state byte (0..255) or -1 for timeout
    int wait_heartbeat(double timeout_s = 2.0) {
        auto deadline = std::chrono::steady_clock::now() + std::chrono::duration<double>(timeout_s);
        while (std::chrono::steady_clock::now() < deadline) {
            auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now()).count();
            if (remaining < 0) remaining = 0;
            auto [ok, frame] = socket_->recv_frame(static_cast<int>(remaining));
            if (!ok) continue;
            uint32_t id = frame.can_id & CAN_SFF_MASK;
            if (id == hb_id_ && frame.can_dlc >= 1) {
                return static_cast<int>(frame.data[0]);
            }
        }
        return -1;
    }

    // SDO write expedited: payload must be 1/2/4 bytes
    void sdo_write(uint16_t index, uint8_t sub, const std::vector<uint8_t>& payload) {
        if (!(payload.size() == 1 || payload.size() == 2 || payload.size() == 4)) {
            throw std::invalid_argument("payload must be 1,2 or 4 bytes for expedited write");
        }
        uint8_t cmd = (payload.size() == 1) ? 0x2F : (payload.size() == 2 ? 0x2B : 0x23);
        struct can_frame req {};
        req.can_id = sdo_tx_;
        req.can_dlc = 8;
        req.data[0] = cmd;
        req.data[1] = static_cast<uint8_t>(index & 0xFF);
        req.data[2] = static_cast<uint8_t>((index >> 8) & 0xFF);
        req.data[3] = static_cast<uint8_t>(sub & 0xFF);
        for (size_t i = 0; i < payload.size(); ++i) req.data[4 + i] = payload[i];
        for (size_t i = payload.size(); i < 4; ++i) req.data[4 + i] = 0;

        socket_->drain(); // clear stale SDO responses
        socket_->send_frame(req);
        auto resp = wait_sdo_resp(index, sub, recv_timeout_s_);
        if (!resp.has_value()) {
            std::ostringstream ss;
            ss << "SDO write timeout 0x" << std::hex << std::setw(4) << std::setfill('0') << index
               << ":" << std::setw(2) << static_cast<int>(sub);
            throw std::runtime_error(ss.str());
        }
        const auto &frame = resp.value();
        if (frame.can_dlc >= 1 && frame.data[0] == 0x80) {
            uint32_t abort = extract_abort_code(frame);
            std::ostringstream ss;
            ss << "SDO abort 0x" << std::hex << std::setw(8) << std::setfill('0') << abort
               << " on write 0x" << std::setw(4) << index << ":" << std::setw(2) << static_cast<int>(sub);
            throw CanopenSdoError(ss.str());
        }
        if (!(frame.can_dlc >= 1 && frame.data[0] == 0x60)) {
            std::ostringstream ss;
            ss << "Unexpected SDO write response: data=" << frame_dump(frame);
            throw CanopenSdoError(ss.str());
        }
    }

    // SDO read expedited: returns bytes (1..4)
    std::vector<uint8_t> sdo_read(uint16_t index, uint8_t sub, double timeout_s = -1.0) {
        if (timeout_s < 0) timeout_s = recv_timeout_s_;
        struct can_frame req {};
        req.can_id = sdo_tx_;
        req.can_dlc = 8;
        req.data[0] = 0x40;
        req.data[1] = static_cast<uint8_t>(index & 0xFF);
        req.data[2] = static_cast<uint8_t>((index >> 8) & 0xFF);
        req.data[3] = sub;
        for (int i = 4; i < 8; ++i) req.data[i] = 0;

        socket_->drain();
        socket_->send_frame(req);
        auto resp = wait_sdo_resp(index, sub, timeout_s);
        if (!resp.has_value()) {
            std::ostringstream ss;
            ss << "SDO read timeout 0x" << std::hex << std::setw(4) << std::setfill('0') << index
               << ":" << std::setw(2) << static_cast<int>(sub);
            throw std::runtime_error(ss.str());
        }
        const auto &frame = resp.value();
        if (frame.can_dlc >= 1 && frame.data[0] == 0x80) {
            uint32_t abort = extract_abort_code(frame);
            std::ostringstream ss;
            ss << "SDO abort 0x" << std::hex << std::setw(8) << std::setfill('0') << abort
               << " on read 0x" << std::setw(4) << index << ":" << std::setw(2) << static_cast<int>(sub);
            throw CanopenSdoError(ss.str());
        }
        if (frame.can_dlc < 4) {
            std::ostringstream ss;
            ss << "SDO read response too short: dlc=" << frame.can_dlc;
            throw std::runtime_error(ss.str());
        }
        uint8_t cmd = frame.data[0];
        if (cmd == 0x4F) {
            return { frame.data[4] };
        } else if (cmd == 0x4B) {
            return { frame.data[4], frame.data[5] };
        } else if (cmd == 0x43) {
            return { frame.data[4], frame.data[5], frame.data[6], frame.data[7] };
        } else {
            std::vector<uint8_t> out;
            for (int i = 4; i < static_cast<int>(frame.can_dlc) && i < 8; ++i) out.push_back(frame.data[i]);
            return out;
        }
    }

    // typed helpers
    void sdo_write_i8(uint16_t index, uint8_t sub, int8_t val) {
        std::vector<uint8_t> p{ static_cast<uint8_t>(val) };
        sdo_write(index, sub, p);
    }
    void sdo_write_u16(uint16_t index, uint8_t sub, uint16_t val) {
        std::vector<uint8_t> p{ static_cast<uint8_t>(val & 0xFF), static_cast<uint8_t>((val >> 8) & 0xFF) };
        sdo_write(index, sub, p);
    }
    int32_t sdo_read_i32(uint16_t index, uint8_t sub) {
        auto b = sdo_read(index, sub);
        if (b.size() >= 4) {
            int32_t v = static_cast<int32_t>(
                (static_cast<uint32_t>(b[0])      ) |
                (static_cast<uint32_t>(b[1]) << 8 ) |
                (static_cast<uint32_t>(b[2]) << 16) |
                (static_cast<uint32_t>(b[3]) << 24));
            return v;
        }
        uint8_t sign_byte = b.empty() ? 0x00 : b.back();
        bool negative = (sign_byte & 0x80) != 0;
        std::vector<uint8_t> b4 = b;
        while (b4.size() < 4) b4.push_back(negative ? 0xFF : 0x00);
        int32_t v = static_cast<int32_t>(
            (static_cast<uint32_t>(b4[0])      ) |
            (static_cast<uint32_t>(b4[1]) << 8 ) |
            (static_cast<uint32_t>(b4[2]) << 16) |
            (static_cast<uint32_t>(b4[3]) << 24));
        return v;
    }
    uint32_t sdo_read_u32(uint16_t index, uint8_t sub) {
        auto b = sdo_read(index, sub);
        if (b.size() >= 4) {
            uint32_t v =
                (static_cast<uint32_t>(b[0])      ) |
                (static_cast<uint32_t>(b[1]) << 8 ) |
                (static_cast<uint32_t>(b[2]) << 16) |
                (static_cast<uint32_t>(b[3]) << 24);
            return v;
        }
        uint32_t v = 0;
        for (size_t i = 0; i < b.size(); ++i) v |= (static_cast<uint32_t>(b[i]) << (8 * i));
        return v;
    }

    // device specific helpers
    void set_velocity_mode() { sdo_write_i8(0x6060, 0x00, 3); }
    void enable_operation() {
        sdo_write_u16(0x6040, 0x00, 0x0006);
        std::this_thread::sleep_for(10ms);
        sdo_write_u16(0x6040, 0x00, 0x0007);
        std::this_thread::sleep_for(10ms);
        sdo_write_u16(0x6040, 0x00, 0x000F);
    }
    void stop() { sdo_write_u16(0x6040, 0x00, 0x0000); }
    void quick_stop() { sdo_write_u16(0x6040, 0x00, 0x0002); }
    void clear_fault() { sdo_write_u16(0x6040, 0x00, 0x0080); }

    // dual motor helpers
    void set_target_speed_lr_rpm(int16_t left_rpm, int16_t right_rpm) {
        std::vector<uint8_t> p(4);
        p[0] = static_cast<uint8_t>(left_rpm & 0xFF); p[1] = static_cast<uint8_t>((left_rpm>>8)&0xFF);
        p[2] = static_cast<uint8_t>(right_rpm & 0xFF); p[3] = static_cast<uint8_t>((right_rpm>>8)&0xFF);
        sdo_write(0x60FF, 0x03, p);
    }
    std::pair<int16_t,int16_t> read_actual_speed_lr_0p1rpm() {
        auto raw = sdo_read(0x606C, 0x03);
        while (raw.size() < 4) raw.push_back(0);
        int16_t left = static_cast<int16_t>((raw[0]) | (raw[1] << 8));
        int16_t right= static_cast<int16_t>((raw[2]) | (raw[3] << 8));
        return {left, right};
    }
    std::pair<int32_t,int32_t> read_encoder_lr() {
        return { sdo_read_i32(0x6064, 0x01), sdo_read_i32(0x6064, 0x02) };
    }
    std::pair<uint16_t,uint16_t> read_statusword_lr() {
        uint32_t sw = sdo_read_u32(0x6041, 0x00);
        return { static_cast<uint16_t>(sw & 0xFFFF), static_cast<uint16_t>((sw >> 16) & 0xFFFF) };
    }
    uint32_t read_fault_code_u32() {
        return sdo_read_u32(0x603F, 0x00);
    }

private:
    int node_id_;
    uint32_t sdo_tx_;
    uint32_t sdo_rx_;
    uint32_t hb_id_;
    double recv_timeout_s_;
    std::shared_ptr<SocketCan> socket_;

    // internal init
    void init_socket(const std::string &channel) {
        // Setup CAN filters: only accept responses from this node's SDO RX and heartbeat
        std::vector<can_filter> filters;
        can_filter f1{static_cast<__u32>(sdo_rx_), 0x7FF};
        can_filter f2{static_cast<__u32>(hb_id_),  0x7FF};
        filters.push_back(f1);
        filters.push_back(f2);
        socket_ = std::make_shared<SocketCan>(channel, filters);
    }

    // wait for SDO response matching index & sub within timeout (seconds)
    std::optional<struct can_frame> wait_sdo_resp(uint16_t index, uint8_t sub, double timeout_s) {
        auto deadline = std::chrono::steady_clock::now() + std::chrono::duration<double>(timeout_s);
        uint8_t idx_lo = static_cast<uint8_t>(index & 0xFF);
        uint8_t idx_hi = static_cast<uint8_t>((index >> 8) & 0xFF);
        while (std::chrono::steady_clock::now() < deadline) {
            auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now()).count();
            if (remaining < 0) remaining = 0;
            auto [ok, frame] = socket_->recv_frame(static_cast<int>(remaining));
            if (!ok) continue;
            uint32_t id = frame.can_id & CAN_SFF_MASK;
            if (id != sdo_rx_) continue;
            if (frame.can_dlc < 4) continue;
            if (frame.data[1] == idx_lo && frame.data[2] == idx_hi && frame.data[3] == sub) {
                return frame;
            }
        }
        return std::nullopt;
    }

    // extract abort code safely (if possible)
    uint32_t extract_abort_code(const struct can_frame &frame) {
        if (frame.can_dlc >= 8) {
            uint32_t abort = (static_cast<uint32_t>(frame.data[4])      ) |
                             (static_cast<uint32_t>(frame.data[5]) << 8 ) |
                             (static_cast<uint32_t>(frame.data[6]) << 16) |
                             (static_cast<uint32_t>(frame.data[7]) << 24);
            return abort;
        }
        return 0;
    }

    static std::string frame_dump(const struct can_frame &f) {
        std::ostringstream ss;
        ss << "can_id=0x" << std::hex << (f.can_id & CAN_SFF_MASK) << " dlc=" << std::dec << int(f.can_dlc) << " data=";
        for (int i = 0; i < f.can_dlc && i < 8; ++i) {
            ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(static_cast<uint8_t>(f.data[i]));
        }
        return ss.str();
    }
};

// ROS2 node that manages two ZLAC8015D instances (front = node 1, rear = node 2)
// and subscribes to 4 Int32 topics to control each wheel independently.
class WheelControllerNode : public rclcpp::Node {
public:
    WheelControllerNode()
        : Node("zlac8015d_controller_node"),
          front_left_cmd_(0), front_right_cmd_(0),
          rear_left_cmd_(0), rear_right_cmd_(0),
          running_(true)
    {
        this->declare_parameter<std::string>("can_interface", "can2");
        this->declare_parameter<double>("control_rate", 2.0);
        
        this->get_parameter("can_interface", can_interface_);
        this->get_parameter("control_rate", control_rate_hz_);

        // allow custom topic names via params
        std::string topic_fl = this->declare_parameter<std::string>("topic_front_left", "/front_left_cmd");
        std::string topic_fr = this->declare_parameter<std::string>("topic_front_right", "/front_right_cmd");
        std::string topic_rl = this->declare_parameter<std::string>("topic_rear_left", "/rear_left_cmd");
        std::string topic_rr = this->declare_parameter<std::string>("topic_rear_right", "/rear_right_cmd");

        sub_fl_ = this->create_subscription<std_msgs::msg::Int32>(
            topic_fl, 1, std::bind(&WheelControllerNode::cbFrontLeft, this, std::placeholders::_1));
        sub_fr_ = this->create_subscription<std_msgs::msg::Int32>(
            topic_fr, 1, std::bind(&WheelControllerNode::cbFrontRight, this, std::placeholders::_1));
        sub_rl_ = this->create_subscription<std_msgs::msg::Int32>(
            topic_rl, 1, std::bind(&WheelControllerNode::cbRearLeft, this, std::placeholders::_1));
        sub_rr_ = this->create_subscription<std_msgs::msg::Int32>(
            topic_rr, 1, std::bind(&WheelControllerNode::cbRearRight, this, std::placeholders::_1));

        // create devices
        RCLCPP_INFO(this->get_logger(), "Creating front and rear ZLAC8015D on interface '%s'...", can_interface_.c_str());
        front_ = std::make_unique<ZLAC8015D>(can_interface_, 1, 0.3);
        rear_  = std::make_unique<ZLAC8015D>(can_interface_, 2, 0.3);

        // initial device setup
        try {
            for (auto *drv : std::vector<ZLAC8015D*>{front_.get(), rear_.get()}) {
                int hb = drv->wait_heartbeat(1.0);
                (void)hb;
                try { drv->clear_fault(); } catch (...) {}
                drv->set_velocity_mode();
                drv->enable_operation();
            }
        } catch (const std::exception &e) {
            RCLCPP_ERROR(this->get_logger(), "Error during initial device setup: %s", e.what());
            throw;
        }
    }

    ~WheelControllerNode() {
        // attempt clean stop
        try {
            if (front_) front_->stop();
            std::this_thread::sleep_for(50ms);
            if (rear_) rear_->stop();
        } catch (...) {}
    }

    void spin() {
        rclcpp::Rate rate(control_rate_hz_);
        while (rclcpp::ok() && running_) {
            rclcpp::spin_some(this->get_node_base_interface());
            // read atomic commands
            int fl = front_left_cmd_.load();
            int fr = front_right_cmd_.load();
            int rl = rear_left_cmd_.load();
            int rr = rear_right_cmd_.load();

            try {
                // set speed via SDO (same as original)
                front_->set_target_speed_lr_rpm(static_cast<int16_t>(fl), static_cast<int16_t>(fr));
                rear_->set_target_speed_lr_rpm(static_cast<int16_t>(rl), static_cast<int16_t>(rr));

                // read back status (best-effort, exceptions caught)
                auto [sp_fl, sp_fr] = front_->read_actual_speed_lr_0p1rpm();
                auto [sp_rl, sp_rr] = rear_->read_actual_speed_lr_0p1rpm();
                auto [enc_fl, enc_fr] = front_->read_encoder_lr();
                auto [enc_rl, enc_rr] = rear_->read_encoder_lr();
                auto [sw_fl, sw_fr] = front_->read_statusword_lr();
                auto [sw_rl, sw_rr] = rear_->read_statusword_lr();
                (void)sp_fl; (void)sp_fr; (void)sp_rl; (void)sp_rr;
                (void)enc_fl; (void)enc_fr; (void)enc_rl; (void)enc_rr;
                (void)sw_fl; (void)sw_fr; (void)sw_rl; (void)sw_rr;

            } catch (const CanopenSdoError &e) {
                RCLCPP_ERROR(this->get_logger(), "CanopenSdoError: %s", e.what());
                // try to print fault codes
                try {
                    uint32_t ffront = front_->read_fault_code_u32();
                    RCLCPP_ERROR(this->get_logger(), "front fault=0x%08x", ffront);
                } catch (...) {}
                try {
                    uint32_t frear = rear_->read_fault_code_u32();
                    RCLCPP_ERROR(this->get_logger(), "rear fault=0x%08x", frear);
                } catch (...) {}
            } catch (const std::exception &e) {
                RCLCPP_WARN(this->get_logger(), "exception: %s", e.what());
            } catch (...) {
                RCLCPP_WARN(this->get_logger(), "unknown exception in control loop");
            }

            rate.sleep();
        }
    }

    void shutdown() {
        running_ = false;
    }

private:
    std::string can_interface_;
    double control_rate_hz_;

    std::unique_ptr<ZLAC8015D> front_;
    std::unique_ptr<ZLAC8015D> rear_;

    // subscribers
    rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr sub_fl_, sub_fr_, sub_rl_, sub_rr_;

    // command variables (atomic)
    std::atomic<int> front_left_cmd_;
    std::atomic<int> front_right_cmd_;
    std::atomic<int> rear_left_cmd_;
    std::atomic<int> rear_right_cmd_;

    std::atomic<bool> running_;

    // callbacks
    void cbFrontLeft(const std_msgs::msg::Int32::SharedPtr msg) { front_left_cmd_.store(msg->data); }
    void cbFrontRight(const std_msgs::msg::Int32::SharedPtr msg) { front_right_cmd_.store(-msg->data); }  // 取反
    void cbRearLeft(const std_msgs::msg::Int32::SharedPtr msg) { rear_left_cmd_.store(msg->data); }
    void cbRearRight(const std_msgs::msg::Int32::SharedPtr msg) { rear_right_cmd_.store(-msg->data); }  // 取反
};


int main(int argc, char **argv) {
    rclcpp::init(argc, argv);

    try {
        auto node = std::make_shared<WheelControllerNode>();
        rclcpp::spin(node);
    } catch (const std::exception &e) {
        RCLCPP_FATAL(rclcpp::get_logger("zlac8015d_controller"), "Fatal error: %s", e.what());
        rclcpp::shutdown();
        return 2;
    }

    RCLCPP_INFO(rclcpp::get_logger("zlac8015d_controller"), "Exiting zlac8015d_controller");
    rclcpp::shutdown();
    return 0;
}

