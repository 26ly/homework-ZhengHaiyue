#include <iostream>
#include <vector>
#include <random>
#include <iomanip>  // 用于格式化输出

struct Point2D {
    double x;
    double y;
};

// 任务1：恒定速度模拟真实位置
std::vector<Point2D> simulateConstantVelocity(
    double total_time, double dt, Point2D initial_pos, Point2D velocity)
{
    int steps = static_cast<int>(total_time / dt);
    std::vector<Point2D> positions;
    positions.reserve(steps + 1);
    positions.push_back(initial_pos);

    for (int i = 1; i <= steps; ++i) {
        Point2D p;
        p.x = initial_pos.x + velocity.x * dt * i;
        p.y = initial_pos.y + velocity.y * dt * i;
        positions.push_back(p);
    }
    return positions;
}

// 任务2：添加测量噪声（均值0，标准差0.5的高斯噪声）
std::vector<Point2D> addMeasurementNoise(
    const std::vector<Point2D>& true_positions, double noise_stddev)
{
    std::default_random_engine generator(std::random_device{}());
    std::normal_distribution<double> noise(0.0, noise_stddev);

    std::vector<Point2D> noisy_positions;
    noisy_positions.reserve(true_positions.size());

    for (const auto& pos : true_positions) {
        Point2D noisy_pos;
        noisy_pos.x = pos.x + noise(generator);
        noisy_pos.y = pos.y + noise(generator);
        noisy_positions.push_back(noisy_pos);
    }
    return noisy_positions;
}

// 任务3：带过程噪声速度模拟位置变化
std::vector<Point2D> simulateWithProcessNoise(
    double total_time, double dt, Point2D initial_pos,
    Point2D initial_velocity, double process_noise_stddev)
{
    int steps = static_cast<int>(total_time / dt);
    std::vector<Point2D> positions;
    positions.reserve(steps + 1);
    positions.push_back(initial_pos);

    Point2D velocity = initial_velocity;
    std::default_random_engine generator(std::random_device{}());
    std::normal_distribution<double> process_noise(0.0, process_noise_stddev);

    for (int i = 1; i <= steps; ++i) {
        // 速度加入过程噪声
        velocity.x += process_noise(generator);
        velocity.y += process_noise(generator);

        // 位置更新
        Point2D p;
        p.x = positions.back().x + velocity.x * dt;
        p.y = positions.back().y + velocity.y * dt;
        positions.push_back(p);
    }

    return positions;
}

int main()
{
    // 输入模拟总时间 t（秒）
    double total_time;
    std::cout << "请输入模拟总时间（单位：秒，建议不大于5秒）：";
    std::cin >> total_time;

    // 时间间隔和初速度定义
    constexpr double dt = 0.01;          // 100fps，每帧10毫秒
    Point2D initial_pos{0.0, 0.0};       // 初始位置 (0,0)
    Point2D initial_velocity{2.0, 3.0};  // 初始速度 (2,3)

    // --- 任务1: 恒定速度真值位置 ---
    auto true_positions = simulateConstantVelocity(total_time, dt, initial_pos, initial_velocity);
    std::cout << "\n--- 任务1：真实位置（恒定速度） ---\n";
    for (size_t i = 0; i < true_positions.size(); ++i) {
        std::cout << "t=" << std::fixed << std::setprecision(3) << i * dt
                  << "s: (" << std::fixed << std::setprecision(4) << true_positions[i].x
                  << ", " << true_positions[i].y << ")\n";
    }

    // --- 任务2: 加测量噪声的观测位置 ---
    constexpr double measurement_noise_stddev = 0.5;
    auto observed_positions = addMeasurementNoise(true_positions, measurement_noise_stddev);
    std::cout << "\n--- 任务2：带测量噪声观测位置 ---\n";
    for (size_t i = 0; i < observed_positions.size(); ++i) {
        std::cout << "t=" << std::fixed << std::setprecision(3) << i * dt
                  << "s: (" << std::fixed << std::setprecision(4) << observed_positions[i].x
                  << ", " << observed_positions[i].y << ")\n";
    }

    // --- 任务3: 带过程噪声的速度模型 ---
    constexpr double process_noise_stddev = 0.1;  // 过程噪声标准差，可根据需求调整
    auto process_noise_positions =
        simulateWithProcessNoise(total_time, dt, initial_pos, initial_velocity, process_noise_stddev);

    std::cout << "\n--- 任务3：带过程噪声速度的真实位置 ---\n";
    for (size_t i = 0; i < process_noise_positions.size(); ++i) {
        std::cout << "t=" << std::fixed << std::setprecision(3) << i * dt
                  << "s: (" << std::fixed << std::setprecision(4) << process_noise_positions[i].x
                  << ", " << process_noise_positions[i].y << ")\n";
    }

    return 0;
}
