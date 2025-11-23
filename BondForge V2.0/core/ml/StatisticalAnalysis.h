#pragma once

#include <vector>
#include <map>
#include <string>
#include <tuple>
#include "../data/DataRecord.h"

namespace BondForge {
namespace Core {
namespace ML {

/**
 * @brief 统计分析结果
 */
struct StatisticalResult {
    double count;
    double sum;
    double mean;
    double median;
    double mode;
    double variance;
    double standardDeviation;
    double min;
    double max;
    double q1;  // 第一四分位数
    double q3;  // 第三四分位数
    double skewness;  // 偏度
    double kurtosis;  // 峰度
};

/**
 * @brief 相关性分析结果
 */
struct CorrelationResult {
    double correlationCoefficient;  // 相关系数
    double pValue;  // p值
    bool isSignificant;  // 是否显著
    double confidenceInterval;  // 置信区间
    std::string interpretation;  // 解释
};

/**
 * @brief 统计分析类
 * 
 * 提供各种统计计算和分析功能
 */
class StatisticalAnalyzer {
public:
    /**
     * @brief 计算基础统计指标
     * 
     * @param data 数据集合
     * @return 统计结果
     */
    static StatisticalResult calculateBasicStatistics(const std::vector<double>& data);
    
    /**
     * @brief 计算两个变量之间的相关性
     * 
     * @param x 第一个变量
     * @param y 第二个变量
     * @param method 计算方法（"pearson", "spearman", "kendall"）
     * @return 相关性结果
     */
    static CorrelationResult calculateCorrelation(
        const std::vector<double>& x, 
        const std::vector<double>& y,
        const std::string& method = "pearson");
    
    /**
     * @brief 对分类数据进行卡方检验
     * 
     * @param observedData 观测数据
     * @param expectedData 期望数据
     * @return 卡方值和p值
     */
    static std::tuple<double, double> chiSquareTest(
        const std::vector<std::vector<int>>& observedData,
        const std::vector<std::vector<double>>& expectedData);
    
    /**
     * @brief t检验
     * 
     * @param group1 第一组数据
     * @param group2 第二组数据
     * @param type 检验类型（"independent", "paired"）
     * @return t值和p值
     */
    static std::tuple<double, double> tTest(
        const std::vector<double>& group1,
        const std::vector<double>& group2,
        const std::string& type = "independent");
    
    /**
     * @brief 方差分析(ANOVA)
     * 
     * @param groups 多组数据
     * @return F值和p值
     */
    static std::tuple<double, double> anova(
        const std::vector<std::vector<double>>& groups);
    
    /**
     * @brief 从数据记录中提取数值序列
     * 
     * @param records 数据记录
     * @param field 要提取的字段
     * @return 数值序列
     */
    static std::vector<double> extractNumericField(
        const std::vector<Data::DataRecord>& records,
        const std::string& field);
    
    /**
     * @brief 从数据记录中按类别分组提取数值序列
     * 
     * @param records 数据记录
     * @param valueField 值字段
     * @param groupField 分组字段
     * @return 分组数据
     */
    static std::map<std::string, std::vector<double>> groupDataByCategory(
        const std::vector<Data::DataRecord>& records,
        const std::string& valueField,
        const std::string& groupField);
    
    /**
     * @brief 生成统计报告
     * 
     * @param records 数据记录
     * @param analysisType 分析类型
     * @return HTML格式的报告
     */
    static std::string generateStatisticalReport(
        const std::vector<Data::DataRecord>& records,
        const std::string& analysisType);
    
private:
    /**
     * @brief 计算中位数
     */
    static double calculateMedian(const std::vector<double>& data);
    
    /**
     * @brief 计算众数
     */
    static double calculateMode(const std::vector<double>& data);
    
    /**
     * @brief 计算四分位数
     */
    static double calculateQuartile(const std::vector<double>& data, double percentile);
    
    /**
     * @brief 计算偏度
     */
    static double calculateSkewness(const std::vector<double>& data);
    
    /**
     * @brief 计算峰度
     */
    static double calculateKurtosis(const std::vector<double>& data);
    
    /**
     * @brief 计算Pearson相关系数
     */
    static CorrelationResult pearsonCorrelation(
        const std::vector<double>& x, 
        const std::vector<double>& y);
    
    /**
     * @brief 计算Spearman秩相关系数
     */
    static CorrelationResult spearmanCorrelation(
        const std::vector<double>& x, 
        const std::vector<double>& y);
};

} // namespace ML
} // namespace Core
} // namespace BondForge