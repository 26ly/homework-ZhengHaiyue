#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

// 自定义异常类
class ImageProcessorException : public std::exception
{
private:
    std::string message;

public:
    ImageProcessorException(const std::string &msg) : message(msg) {}
    const char *what() const noexcept override
    {
        return message.c_str();
    }
};

// 图像处理工具类
class ImageProcessor
{
private:
    cv::Mat image;
    std::string imagePath;

public:
    // 构造函数 - 初始化并加载图像
    ImageProcessor(const std::string &path) : imagePath(path)
    {
        try
        {
            image = cv::imread(path);
            if (image.empty())
            {
                throw ImageProcessorException("无法加载图像: " + path + " (可能路径无效或格式不支持)");
            }
            std::cout << "成功加载图像: " << path << std::endl;
        }
        catch (const cv::Exception &e)
        {
            throw ImageProcessorException("OpenCV错误: " + std::string(e.what()));
        }
    }

    // 获取图像尺寸
    cv::Size getImageSize() const
    {
        if (image.empty())
        {
            throw ImageProcessorException("图像为空，无法获取尺寸");
        }
        return image.size();
    }

    // 获取通道数
    int getChannels() const
    {
        if (image.empty())
        {
            throw ImageProcessorException("图像为空，无法获取通道数");
        }
        return image.channels();
    }

    // 获取像素数据
    cv::Mat getPixelData() const
    {
        if (image.empty())
        {
            throw ImageProcessorException("图像为空，无法获取像素数据");
        }
        return image.clone();
    }

    // 预处理功能1: RGB转灰度图
    cv::Mat convertToGray() const
    {
        if (image.empty())
        {
            throw ImageProcessorException("图像为空，无法转换为灰度图");
        }
        cv::Mat grayImage;
        cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
        return grayImage;
    }

    // 预处理功能2: 均值模糊去噪
    cv::Mat applyMeanBlur(int kernelSize = 5) const
    {
        if (image.empty())
        {
            throw ImageProcessorException("图像为空，无法应用均值模糊");
        }
        if (kernelSize <= 0 || kernelSize % 2 == 0)
        {
            throw ImageProcessorException("核大小必须为正奇数");
        }
        cv::Mat blurredImage;
        cv::blur(image, blurredImage, cv::Size(kernelSize, kernelSize));
        return blurredImage;
    }

    // 预处理功能3: 高斯模糊
    cv::Mat applyGaussianBlur(int kernelSize = 5, double sigmaX = 1.0) const
    {
        if (image.empty())
        {
            throw ImageProcessorException("图像为空，无法应用高斯模糊");
        }
        if (kernelSize <= 0 || kernelSize % 2 == 0)
        {
            throw ImageProcessorException("核大小必须为正奇数");
        }
        cv::Mat gaussianBlurred;
        cv::GaussianBlur(image, gaussianBlurred, cv::Size(kernelSize, kernelSize), sigmaX);
        return gaussianBlurred;
    }

    // 灯条阈值分割功能
    cv::Mat extractLightBars() const
    {
        if (image.empty())
        {
            throw ImageProcessorException("图像为空，无法提取灯条");
        }

        // 分离BGR通道
        std::vector<cv::Mat> bgr_channels;
        cv::split(image, bgr_channels);

        // 提取红色通道（装甲板灯条通常为红色或蓝色，这里以红色为例）
        cv::Mat red_channel = bgr_channels[2];  // BGR格式中，索引2为红色通道
        cv::Mat blue_channel = bgr_channels[0]; // BGR格式中，索引0为蓝色通道

        // 创建HSV图像用于更好的颜色分割
        cv::Mat hsv_image;
        cv::cvtColor(image, hsv_image, cv::COLOR_BGR2HSV);

        // 定义红色HSV范围
        cv::Scalar red_lower1(0, 100, 100);
        cv::Scalar red_upper1(10, 255, 255);
        cv::Scalar red_lower2(160, 100, 100);
        cv::Scalar red_upper2(180, 255, 255);

        // 定义蓝色HSV范围
        cv::Scalar blue_lower(100, 100, 100);
        cv::Scalar blue_upper(130, 255, 255);

        // 创建红色和蓝色掩码
        cv::Mat red_mask1, red_mask2, red_mask, blue_mask, final_mask;
        cv::inRange(hsv_image, red_lower1, red_upper1, red_mask1);
        cv::inRange(hsv_image, red_lower2, red_upper2, red_mask2);
        cv::inRange(hsv_image, blue_lower, blue_upper, blue_mask);

        // 合并红色掩码
        red_mask = red_mask1 | red_mask2;

        // 合并红色和蓝色掩码
        final_mask = red_mask | blue_mask;

        // 形态学操作，去除噪声
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        cv::morphologyEx(final_mask, final_mask, cv::MORPH_OPEN, kernel);
        cv::morphologyEx(final_mask, final_mask, cv::MORPH_CLOSE, kernel);

        std::cout << "成功提取灯条候选区域" << std::endl;
        return final_mask;
    }

    // 提高任务：筛选符合装甲板灯条特征的目标
    cv::Mat filterLightBars(const cv::Mat &binaryImage, cv::Mat &visualResult) const
    {
        if (binaryImage.empty())
        {
            throw ImageProcessorException("二值化图像为空");
        }

        // 复制原图用于可视化
        visualResult = image.clone();

        // 查找轮廓
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(binaryImage, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        std::vector<cv::Rect> validLightBars;

        std::cout << "找到 " << contours.size() << " 个轮廓" << std::endl;

        for (size_t i = 0; i < contours.size(); i++)
        {
            // 计算外接矩形
            cv::Rect boundingRect = cv::boundingRect(contours[i]);

            // 计算面积
            double area = cv::contourArea(contours[i]);

            // 计算长宽比
            double aspectRatio = (double)boundingRect.height / boundingRect.width;

            // 设定筛选条件（根据装甲板灯条特征调整）
            bool isValidLightBar = (area > 50 &&              // 面积阈值
                                    area < 5000 &&            // 最大面积限制
                                    aspectRatio > 1.5 &&      // 灯条通常较细长
                                    aspectRatio < 8.0 &&      // 不能过于细长
                                    boundingRect.width > 3 && // 最小宽度
                                    boundingRect.height > 10  // 最小高度
            );

            if (isValidLightBar)
            {
                validLightBars.push_back(boundingRect);

                // 在原图上标记
                cv::rectangle(visualResult, boundingRect, cv::Scalar(0, 255, 0), 2);

                // 添加文本信息
                std::string info = "A:" + std::to_string((int)area) +
                                   " R:" + std::to_string(aspectRatio).substr(0, 4);
                cv::putText(visualResult, info,
                            cv::Point(boundingRect.x, boundingRect.y - 5),
                            cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 255, 0), 1);

                std::cout << "有效灯条 " << validLightBars.size()
                          << ": 面积=" << area
                          << ", 长宽比=" << aspectRatio
                          << ", 位置=(" << boundingRect.x << "," << boundingRect.y << ")" << std::endl;
            }
        }

        std::cout << "筛选出 " << validLightBars.size() << " 个有效灯条" << std::endl;
        return visualResult;
    }

    // 显示图像信息
    void displayImageInfo() const
    {
        if (image.empty())
        {
            std::cout << "图像为空" << std::endl;
            return;
        }

        cv::Size size = getImageSize();
        std::cout << "=== 图像信息 ===" << std::endl;
        std::cout << "路径: " << imagePath << std::endl;
        std::cout << "尺寸: " << size.width << " x " << size.height << std::endl;
        std::cout << "通道数: " << getChannels() << std::endl;
        std::cout << "总像素数: " << size.width * size.height << std::endl;
    }
};

int main()
{
    try
    {
        // 1. 初始化图像处理器
        std::cout << "=== OpenCV装甲板灯条检测 ===" << std::endl;

        // 图像路径
        std::string imagePath = "hero.png";
        ImageProcessor processor(imagePath);

        // 2. 显示基础信息
        processor.displayImageInfo();

        // 3. 预处理演示
        std::cout << "\n=== 预处理功能演示 ===" << std::endl;

        // 转换为灰度图
        cv::Mat grayImage = processor.convertToGray();
        std::cout << "✓ 成功转换为灰度图" << std::endl;

        // 均值模糊
        cv::Mat blurredImage = processor.applyMeanBlur(5);
        std::cout << "✓ 成功应用均值模糊" << std::endl;

        // 高斯模糊
        cv::Mat gaussianBlurred = processor.applyGaussianBlur(5, 1.0);
        std::cout << "✓ 成功应用高斯模糊" << std::endl;

        // 4. 灯条检测
        std::cout << "\n=== 灯条检测 ===" << std::endl;
        cv::Mat lightBarMask = processor.extractLightBars();

        // 5. 提高任务：筛选有效灯条
        std::cout << "\n=== 灯条筛选与可视化 ===" << std::endl;
        cv::Mat visualResult;
        cv::Mat filteredResult = processor.filterLightBars(lightBarMask, visualResult);

        // 6. 显示结果
        std::cout << "\n=== 显示结果 ===" << std::endl;

        // 显示原图
        cv::namedWindow("原始图像", cv::WINDOW_AUTOSIZE);
        cv::imshow("原始图像", processor.getPixelData());

        // 显示灰度图
        cv::namedWindow("灰度图", cv::WINDOW_AUTOSIZE);
        cv::imshow("灰度图", grayImage);

        // 显示模糊后图像
        cv::namedWindow("均值模糊", cv::WINDOW_AUTOSIZE);
        cv::imshow("均值模糊", blurredImage);

        // 显示二值化结果
        cv::namedWindow("灯条二值化", cv::WINDOW_AUTOSIZE);
        cv::imshow("灯条二值化", lightBarMask);

        // 显示标记结果
        cv::namedWindow("灯条检测结果", cv::WINDOW_AUTOSIZE);
        cv::imshow("灯条检测结果", visualResult);

        // 7. 保存结果
        cv::imwrite("output_gray.jpg", grayImage);
        cv::imwrite("output_blur.jpg", blurredImage);
        cv::imwrite("output_lightbar_mask.jpg", lightBarMask);
        cv::imwrite("output_result.jpg", visualResult);
        std::cout << "✓ 结果已保存到当前目录" << std::endl;

        std::cout << "\n按任意键退出..." << std::endl;
        cv::waitKey(0);
        cv::destroyAllWindows();
    }
    catch (const ImageProcessorException &e)
    {
        std::cerr << "图像处理错误: " << e.what() << std::endl;
        return -1;
    }
    catch (const cv::Exception &e)
    {
        std::cerr << "OpenCV错误: " << e.what() << std::endl;
        return -1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "标准错误: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}