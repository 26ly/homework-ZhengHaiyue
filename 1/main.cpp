#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <stdexcept>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Matrix2D {
private:
    std::vector<std::vector<double>> data;
    int rows, cols;

public:
    Matrix2D(int r, int c) : rows(r), cols(c) {
        data.resize(rows, std::vector<double>(cols, 0.0));
    }
    Matrix2D(std::initializer_list<std::initializer_list<double>> init) {
        rows = init.size();
        cols = init.begin()->size();
        data.resize(rows, std::vector<double>(cols));
        
        int i = 0;
        for (const auto& row : init) {
            if (row.size() != cols) {
                throw std::invalid_argument("矩阵行数不一致");
            }
            int j = 0;
            for (const auto& val : row) {
                data[i][j] = val;
                j++;
            }
            i++;
        }
    }
    double& operator()(int i, int j) {
        if (i >= rows || j >= cols || i < 0 || j < 0) {
            throw std::out_of_range("矩阵索引越界");
        }
        return data[i][j];
    }
    
    const double& operator()(int i, int j) const {
        if (i >= rows || j >= cols || i < 0 || j < 0) {
            throw std::out_of_range("矩阵索引越界");
        }
        return data[i][j];
    }
    Matrix2D operator+(const Matrix2D& other) const {
        if (rows != other.rows || cols != other.cols) {
            throw std::invalid_argument("矩阵维度不匹配，无法相加");
        }
        
        Matrix2D result(rows, cols);
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                result(i, j) = data[i][j] + other(i, j);
            }
        }
        return result;
    }
    Matrix2D operator*(const Matrix2D& other) const {
        if (cols != other.rows) {
            throw std::invalid_argument("矩阵维度不匹配，无法相乘");
        }
        
        Matrix2D result(rows, other.cols);
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < other.cols; j++) {
                for (int k = 0; k < cols; k++) {
                    result(i, j) += data[i][k] * other(k, j);
                }
            }
        }
        return result;
    }
    int getRows() const { return rows; }
    int getCols() const { return cols; }
    void display() const {
        std::cout << std::fixed << std::setprecision(3);
        for (int i = 0; i < rows; i++) {
            std::cout << "[ ";
            for (int j = 0; j < cols; j++) {
                std::cout << std::setw(8) << data[i][j];
                if (j < cols - 1) std::cout << ", ";
            }
            std::cout << " ]" << std::endl;
        }
        std::cout << std::endl;
    }
};

class Point2D {
public:
    double x, y;
    
    Point2D(double x = 0, double y = 0) : x(x), y(y) {}

    Matrix2D toHomogeneous() const {
        return Matrix2D{{x}, {y}, {1}};
    }
    
    void display() const {
        std::cout << "Point(" << x << ", " << y << ")" << std::endl;
    }
};

class Transform2D {
public:
    static Matrix2D translation(double tx, double ty) {
        return Matrix2D{
            {1, 0, tx},
            {0, 1, ty},
            {0, 0, 1}
        };
    }
    
    static Matrix2D rotation(double angle) {
        double cosA = std::cos(angle);
        double sinA = std::sin(angle);
        return Matrix2D{
            {cosA, -sinA, 0},
            {sinA,  cosA, 0},
            {0,     0,    1}
        };
    }
    
    static Matrix2D scaling(double sx, double sy) {
        return Matrix2D{
            {sx, 0,  0},
            {0,  sy, 0},
            {0,  0,  1}
        };
    }
    static Matrix2D rotateAndTranslate(double angle, double tx, double ty) {
        return translation(tx, ty) * rotation(angle);
    }
};

Point2D extractPoint(const Matrix2D& homogeneous) {
    if (homogeneous.getRows() != 3 || homogeneous.getCols() != 1) {
        throw std::invalid_argument("无效的齐次坐标格式");
    }
    return Point2D(homogeneous(0, 0), homogeneous(1, 0));
}

int main() {
    try {
        std::cout << "=== 二维矩阵变换演示程序 ===" << std::endl << std::endl;

        Point2D originalPoint(3.0, 4.0);
        std::cout << "原始点: ";
        originalPoint.display();
        std::cout << std::endl;
        
        Matrix2D pointMatrix = originalPoint.toHomogeneous();
        std::cout << "齐次坐标表示:" << std::endl;
        pointMatrix.display();
        
        std::cout << "1. 平移变换 (tx=2, ty=3):" << std::endl;
        Matrix2D translationMatrix = Transform2D::translation(2.0, 3.0);
        std::cout << "平移矩阵:" << std::endl;
        translationMatrix.display();
        
        Matrix2D translatedPoint = translationMatrix * pointMatrix;
        Point2D result1 = extractPoint(translatedPoint);
        std::cout << "变换后的点: ";
        result1.display();
        std::cout << std::endl;

        std::cout << "2. 旋转变换 (角度=π/4):" << std::endl;
        double angle = M_PI / 4.0; 
        Matrix2D rotationMatrix = Transform2D::rotation(angle);
        std::cout << "旋转矩阵:" << std::endl;
        rotationMatrix.display();
        
        Matrix2D rotatedPoint = rotationMatrix * pointMatrix;
        Point2D result2 = extractPoint(rotatedPoint);
        std::cout << "变换后的点: ";
        result2.display();
        std::cout << std::endl;

        std::cout << "3. 缩放变换 (sx=2, sy=1.5):" << std::endl;
        Matrix2D scalingMatrix = Transform2D::scaling(2.0, 1.5);
        std::cout << "缩放矩阵:" << std::endl;
        scalingMatrix.display();
        
        Matrix2D scaledPoint = scalingMatrix * pointMatrix;
        Point2D result3 = extractPoint(scaledPoint);
        std::cout << "变换后的点: ";
        result3.display();
        std::cout << std::endl;
        
        std::cout << "4. 组合变换 (先旋转π/6，再平移(1,2)):" << std::endl;
        Matrix2D combinedMatrix = Transform2D::rotateAndTranslate(M_PI/6, 1.0, 2.0);
        std::cout << "组合变换矩阵:" << std::endl;
        combinedMatrix.display();
        
        Matrix2D combinedResult = combinedMatrix * pointMatrix;
        Point2D result4 = extractPoint(combinedResult);
        std::cout << "变换后的点: ";
        result4.display();
        std::cout << std::endl;
        
        std::cout << "5. 矩阵加法演示:" << std::endl;
        Matrix2D matA{{1, 2}, {3, 4}};
        Matrix2D matB{{5, 6}, {7, 8}};
        
        std::cout << "矩阵A:" << std::endl;
        matA.display();
        std::cout << "矩阵B:" << std::endl;
        matB.display();
        
        Matrix2D matSum = matA + matB;
        std::cout << "A + B = " << std::endl;
        matSum.display();
        
        Matrix2D matProduct = matA * matB;
        std::cout << "A * B = " << std::endl;
        matProduct.display();
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
