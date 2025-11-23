#include "StatisticalAnalysis.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>
#include <unordered_map>
#include <boost/math/distributions/normal.hpp>

namespace BondForge {
namespace Core {
namespace ML {

// StatisticalAnalyzer 实现
StatisticalResult StatisticalAnalyzer::calculateBasicStatistics(const std::vector<double>& data) {
    StatisticalResult result;
    
    if (data.empty()) {
        return result; // 返回默认值
    }
    
    // 复制数据以便排序
    std::vector<double> sortedData = data;
    std::sort(sortedData.begin(), sortedData.end());
    
    // 基础统计量
    result.count = static_cast<double>(data.size());
    result.sum = std::accumulate(data.begin(), data.end(), 0.0);
    result.mean = result.sum / result.count;
    result.min = sortedData.front();
    result.max = sortedData.back();
    
    // 中位数
    result.median = calculateMedian(sortedData);
    
    // 众数
    result.mode = calculateMode(sortedData);
    
    // 四分位数
    result.q1 = calculateQuartile(sortedData, 0.25);
    result.q3 = calculateQuartile(sortedData, 0.75);
    
    // 方差和标准差
    double sumSquaredDiffs = 0.0;
    for (double value : data) {
        double diff = value - result.mean;
        sumSquaredDiffs += diff * diff;
    }
    result.variance = sumSquaredDiffs / result.count;
    result.standardDeviation = std::sqrt(result.variance);
    
    // 偏度和峰度
    result.skewness = calculateSkewness(data);
    result.kurtosis = calculateKurtosis(data);
    
    return result;
}

CorrelationResult StatisticalAnalyzer::calculateCorrelation(
    const std::vector<double>& x, 
    const std::vector<double>& y,
    const std::string& method) {
    
    if (method == "pearson") {
        return pearsonCorrelation(x, y);
    } else if (method == "spearman") {
        return spearmanCorrelation(x, y);
    }
    
    // 默认使用Pearson相关系数
    return pearsonCorrelation(x, y);
}

std::tuple<double, double> StatisticalAnalyzer::chiSquareTest(
    const std::vector<std::vector<int>>& observedData,
    const std::vector<std::vector<double>>& expectedData) {
    
    // 计算卡方值
    double chiSquare = 0.0;
    for (size_t i = 0; i < observedData.size(); ++i) {
        for (size_t j = 0; j < observedData[i].size(); ++j) {
            double observed = observedData[i][j];
            double expected = expectedData[i][j];
            
            if (expected > 0) {
                chiSquare += (observed - expected) * (observed - expected) / expected;
            }
        }
    }
    
    // 计算自由度
    size_t df = (observedData.size() - 1) * (observedData[0].size() - 1);
    
    // 计算p值（使用近似方法）
    // 这里使用近似公式，实际应用中应使用专门的统计库
    double pValue = 0.0;
    if (df > 0) {
        // 使用正态分布近似
        boost::math::normal_distribution<> dist(0.0, 1.0);
        pValue = 1.0 - boost::math::cdf(dist, std::sqrt(2.0 * chiSquare));
    }
    
    return std::make_tuple(chiSquare, pValue);
}

std::tuple<double, double> StatisticalAnalyzer::tTest(
    const std::vector<double>& group1,
    const std::vector<double>& group2,
    const std::string& type) {
    
    if (group1.empty() || group2.empty()) {
        return std::make_tuple(0.0, 1.0);
    }
    
    // 计算两组数据的均值和方差
    double mean1 = std::accumulate(group1.begin(), group1.end(), 0.0) / group1.size();
    double mean2 = std::accumulate(group2.begin(), group2.end(), 0.0) / group2.size();
    
    double var1 = 0.0, var2 = 0.0;
    for (double value : group1) {
        var1 += (value - mean1) * (value - mean1);
    }
    var1 /= (group1.size() - 1);
    
    for (double value : group2) {
        var2 += (value - mean2) * (value - mean2);
    }
    var2 /= (group2.size() - 1);
    
    double tValue, degreesOfFreedom;
    
    if (type == "independent") {
        // 独立样本t检验
        double pooledVariance = ((group1.size() - 1) * var1 + (group2.size() - 1) * var2) / 
                             (group1.size() + group2.size() - 2);
        double standardError = std::sqrt(pooledVariance * (1.0 / group1.size() + 1.0 / group2.size()));
        tValue = (mean1 - mean2) / standardError;
        degreesOfFreedom = group1.size() + group2.size() - 2;
    } else {
        // 配对样本t检验
        if (group1.size() != group2.size()) {
            return std::make_tuple(0.0, 1.0);
        }
        
        std::vector<double> differences;
        for (size_t i = 0; i < group1.size(); ++i) {
            differences.push_back(group1[i] - group2[i]);
        }
        
        double meanDiff = std::accumulate(differences.begin(), differences.end(), 0.0) / differences.size();
        double varDiff = 0.0;
        for (double diff : differences) {
            varDiff += (diff - meanDiff) * (diff - meanDiff);
        }
        varDiff /= (differences.size() - 1);
        
        tValue = meanDiff / (std::sqrt(varDiff / differences.size()));
        degreesOfFreedom = differences.size() - 1;
    }
    
    // 计算p值（使用近似方法）
    // 这里使用近似公式，实际应用中应使用专门的统计库
    boost::math::students_t_distribution<> dist(degreesOfFreedom);
    double pValue = 2.0 * (1.0 - boost::math::cdf(dist, std::abs(tValue)));
    
    return std::make_tuple(tValue, pValue);
}

std::tuple<double, double> StatisticalAnalyzer::anova(
    const std::vector<std::vector<double>>& groups) {
    
    if (groups.size() < 2) {
        return std::make_tuple(0.0, 1.0);
    }
    
    // 计算总均值
    size_t totalSize = 0;
    double totalSum = 0.0;
    for (const auto& group : groups) {
        totalSize += group.size();
        totalSum += std::accumulate(group.begin(), group.end(), 0.0);
    }
    double grandMean = totalSum / totalSize;
    
    // 计算组间方差(Between-group variance)
    double ssBetween = 0.0;
    for (const auto& group : groups) {
        if (!group.empty()) {
            double groupMean = std::accumulate(group.begin(), group.end(), 0.0) / group.size();
            ssBetween += group.size() * (groupMean - grandMean) * (groupMean - grandMean);
        }
    }
    double msBetween = ssBetween / (groups.size() - 1);
    
    // 计算组内方差(Within-group variance)
    double ssWithin = 0.0;
    for (const auto& group : groups) {
        if (!group.empty()) {
            double groupMean = std::accumulate(group.begin(), group.end(), 0.0) / group.size();
            for (double value : group) {
                ssWithin += (value - groupMean) * (value - groupMean);
            }
        }
    }
    double msWithin = ssWithin / (totalSize - groups.size());
    
    // 计算F值
    double fValue = msWithin > 0 ? msBetween / msWithin : 0.0;
    
    // 计算p值（使用近似方法）
    boost::math::fisher_f_distribution<> dist(groups.size() - 1, totalSize - groups.size());
    double pValue = 1.0 - boost::math::cdf(dist, fValue);
    
    return std::make_tuple(fValue, pValue);
}

std::vector<double> StatisticalAnalyzer::extractNumericField(
    const std::vector<Data::DataRecord>& records,
    const std::string& field) {
    
    std::vector<double> values;
    
    if (field == "content_length") {
        for (const auto& record : records) {
            values.push_back(static_cast<double>(record.content.length()));
        }
    } 
    else if (field == "timestamp") {
        for (const auto& record : records) {
            values.push_back(static_cast<double>(record.timestamp));
        }
    } 
    else if (field == "tag_count") {
        for (const auto& record : records) {
            values.push_back(static_cast<double>(record.tags.size()));
        }
    }
    
    return values;
}

std::map<std::string, std::vector<double>> StatisticalAnalyzer::groupDataByCategory(
    const std::vector<Data::DataRecord>& records,
    const std::string& valueField,
    const std::string& groupField) {
    
    std::map<std::string, std::vector<double>> groupedData;
    
    for (const auto& record : records) {
        std::string group;
        
        if (groupField == "category") {
            group = record.category;
        } 
        else if (groupField == "format") {
            group = record.format;
        } 
        else if (groupField == "uploader") {
            group = record.uploader;
        }
        
        double value = 0.0;
        if (valueField == "content_length") {
            value = static_cast<double>(record.content.length());
        } 
        else if (valueField == "timestamp") {
            value = static_cast<double>(record.timestamp);
        } 
        else if (valueField == "tag_count") {
            value = static_cast<double>(record.tags.size());
        }
        
        groupedData[group].push_back(value);
    }
    
    return groupedData;
}

std::string StatisticalAnalyzer::generateStatisticalReport(
    const std::vector<Data::DataRecord>& records,
    const std::string& analysisType) {
    
    std::stringstream html;
    html << "<!DOCTYPE html>\n";
    html << "<html>\n<head>\n";
    html << "<title>Statistical Analysis Report</title>\n";
    html << "<style>\n";
    html << "body { font-family: Arial, sans-serif; margin: 20px; }\n";
    html << "h1 { color: #2c3e50; }\n";
    html << "h2 { color: #3498db; border-bottom: 1px solid #ddd; padding-bottom: 5px; }\n";
    html << "table { border-collapse: collapse; width: 100%; margin-bottom: 20px; }\n";
    html << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n";
    html << "th { background-color: #f2f2f2; }\n";
    html << "tr:nth-child(even) { background-color: #f9f9f9; }\n";
    html << ".significant { color: #e74c3c; font-weight: bold; }\n";
    html << ".not-significant { color: #27ae60; }\n";
    html << "</style>\n";
    html << "</head>\n<body>\n";
    
    html << "<h1>BondForge Statistical Analysis Report</h1>\n";
    html << "<p>Generated on: " << std::chrono::system_clock::now().time_since_epoch().count() << "</p>\n";
    
    if (analysisType == "overall") {
        html << "<h2>Overall Statistics</h2>\n";
        
        // 总体数据量
        html << "<p>Total records: " << records.size() << "</p>\n";
        
        // 按类别统计
        std::map<std::string, int> categoryCounts;
        for (const auto& record : records) {
            categoryCounts[record.category]++;
        }
        
        html << "<h3>Records by Category</h3>\n";
        html << "<table>\n";
        html << "<tr><th>Category</th><th>Count</th><th>Percentage</th></tr>\n";
        
        for (const auto& pair : categoryCounts) {
            double percentage = 100.0 * pair.second / records.size();
            html << "<tr><td>" << pair.first << "</td><td>" << pair.second 
                  << "</td><td>" << std::fixed << std::setprecision(2) << percentage 
                  << "%</td></tr>\n";
        }
        
        html << "</table>\n";
        
        // 内容长度统计
        auto contentLengths = extractNumericField(records, "content_length");
        auto contentStats = calculateBasicStatistics(contentLengths);
        
        html << "<h3>Content Length Statistics</h3>\n";
        html << "<table>\n";
        html << "<tr><th>Metric</th><th>Value</th></tr>\n";
        html << "<tr><td>Count</td><td>" << contentStats.count << "</td></tr>\n";
        html << "<tr><td>Mean</td><td>" << std::fixed << std::setprecision(2) 
              << contentStats.mean << "</td></tr>\n";
        html << "<tr><td>Median</td><td>" << std::fixed << std::setprecision(2) 
              << contentStats.median << "</td></tr>\n";
        html << "<tr><td>Standard Deviation</td><td>" << std::fixed << std::setprecision(2) 
              << contentStats.standardDeviation << "</td></tr>\n";
        html << "<tr><td>Minimum</td><td>" << contentStats.min << "</td></tr>\n";
        html << "<tr><td>Maximum</td><td>" << contentStats.max << "</td></tr>\n";
        html << "</table>\n";
        
    } 
    else if (analysisType == "correlation") {
        html << "<h2>Correlation Analysis</h2>\n";
        
        // 内容长度与时间戳的相关性
        auto contentLengths = extractNumericField(records, "content_length");
        auto timestamps = extractNumericField(records, "timestamp");
        
        auto correlation = calculateCorrelation(contentLengths, timestamps);
        
        html << "<h3>Content Length vs. Timestamp</h3>\n";
        html << "<p>Correlation Coefficient: " << std::fixed << std::setprecision(4) 
              << correlation.correlationCoefficient << "</p>\n";
        html << "<p>P-value: " << std::fixed << std::setprecision(6) 
              << correlation.pValue << "</p>\n";
        
        html << "<p class=\"" << (correlation.isSignificant ? "significant" : "not-significant") << "\">";
        html << "Significance: " << (correlation.isSignificant ? "Significant" : "Not Significant");
        html << "</p>\n";
        
        html << "<p>Interpretation: " << correlation.interpretation << "</p>\n";
        
    }
    
    html << "</body>\n</html>";
    
    return html.str();
}

// 私有辅助方法实现
double StatisticalAnalyzer::calculateMedian(const std::vector<double>& data) {
    if (data.empty()) return 0.0;
    
    size_t size = data.size();
    if (size % 2 == 0) {
        return (data[size/2 - 1] + data[size/2]) / 2.0;
    } else {
        return data[size/2];
    }
}

double StatisticalAnalyzer::calculateMode(const std::vector<double>& data) {
    if (data.empty()) return 0.0;
    
    std::unordered_map<double, int> frequency;
    for (double value : data) {
        frequency[value]++;
    }
    
    double mode = data[0];
    int maxCount = frequency[mode];
    
    for (const auto& pair : frequency) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
            mode = pair.first;
        }
    }
    
    return mode;
}

double StatisticalAnalyzer::calculateQuartile(const std::vector<double>& data, double percentile) {
    if (data.empty()) return 0.0;
    
    double index = percentile * (data.size() - 1);
    int lowerIndex = static_cast<int>(std::floor(index));
    int upperIndex = static_cast<int>(std::ceil(index));
    
    if (lowerIndex == upperIndex) {
        return data[lowerIndex];
    }
    
    double weight = index - lowerIndex;
    return data[lowerIndex] * (1 - weight) + data[upperIndex] * weight;
}

double StatisticalAnalyzer::calculateSkewness(const std::vector<double>& data) {
    if (data.size() < 3) return 0.0;
    
    double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    
    double sum = 0.0;
    for (double value : data) {
        double diff = value - mean;
        sum += diff * diff * diff;
    }
    
    double variance = sum / (data.size() - 1);
    double stdDev = std::sqrt(variance);
    
    if (stdDev == 0.0) return 0.0;
    
    double skewSum = 0.0;
    for (double value : data) {
        double diff = value - mean;
        skewSum += diff * diff * diff / (stdDev * stdDev * stdDev);
    }
    
    return skewSum / data.size();
}

double StatisticalAnalyzer::calculateKurtosis(const std::vector<double>& data) {
    if (data.size() < 4) return 0.0;
    
    double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    
    double sum = 0.0;
    for (double value : data) {
        double diff = value - mean;
        sum += diff * diff;
    }
    
    double variance = sum / (data.size() - 1);
    double stdDev = std::sqrt(variance);
    
    if (stdDev == 0.0) return 0.0;
    
    double kurtSum = 0.0;
    for (double value : data) {
        double diff = value - mean;
        kurtSum += diff * diff * diff * diff * diff / (stdDev * stdDev * stdDev * stdDev * stdDev);
    }
    
    return kurtSum / data.size() - 3.0; // 减去3得到超额峰度
}

CorrelationResult StatisticalAnalyzer::pearsonCorrelation(
    const std::vector<double>& x, 
    const std::vector<double>& y) {
    
    if (x.size() != y.size() || x.size() < 2) {
        return {0.0, 1.0, false, 0.0, "Insufficient data"};
    }
    
    // 计算均值
    double meanX = std::accumulate(x.begin(), x.end(), 0.0) / x.size();
    double meanY = std::accumulate(y.begin(), y.end(), 0.0) / y.size();
    
    // 计算协方差和方差
    double sumXY = 0.0, sumX2 = 0.0, sumY2 = 0.0;
    
    for (size_t i = 0; i < x.size(); ++i) {
        double diffX = x[i] - meanX;
        double diffY = y[i] - meanY;
        sumXY += diffX * diffY;
        sumX2 += diffX * diffX;
        sumY2 += diffY * diffY;
    }
    
    // 计算相关系数
    double denominator = std::sqrt(sumX2 * sumY2);
    double correlation = denominator > 0 ? sumXY / denominator : 0.0;
    
    // 确保相关系数在[-1, 1]范围内
    correlation = std::max(-1.0, std::min(1.0, correlation));
    
    // 计算t值和p值
    double df = x.size() - 2;
    double tValue = df > 0 ? correlation * std::sqrt(df / (1 - correlation * correlation)) : 0.0;
    
    // 使用近似方法计算p值
    boost::math::students_t_distribution<> dist(df);
    double pValue = 2.0 * (1.0 - boost::math::cdf(dist, std::abs(tValue)));
    
    // 判断是否显著（α = 0.05）
    bool isSignificant = pValue < 0.05;
    
    // 置信区间（简化）
    double confidenceInterval = 1.96 * std::sqrt((1 - correlation * correlation) / (x.size() - 2));
    
    // 解释
    std::string interpretation;
    if (std::abs(correlation) < 0.3) {
        interpretation = "Weak correlation";
    } else if (std::abs(correlation) < 0.5) {
        interpretation = "Moderate correlation";
    } else if (std::abs(correlation) < 0.7) {
        interpretation = "Strong correlation";
    } else {
        interpretation = "Very strong correlation";
    }
    
    if (correlation < 0) {
        interpretation += " (negative)";
    } else {
        interpretation += " (positive)";
    }
    
    return {correlation, pValue, isSignificant, confidenceInterval, interpretation};
}

CorrelationResult StatisticalAnalyzer::spearmanCorrelation(
    const std::vector<double>& x, 
    const std::vector<double>& y) {
    
    if (x.size() != y.size() || x.size() < 2) {
        return {0.0, 1.0, false, 0.0, "Insufficient data"};
    }
    
    // 创建副本以便排序
    std::vector<std::pair<double, size_t>> xRanked, yRanked;
    
    // 计算x的秩
    for (size_t i = 0; i < x.size(); ++i) {
        xRanked.emplace_back(x[i], i);
    }
    std::sort(xRanked.begin(), xRanked.end());
    
    std::vector<double> xRanks(x.size());
    for (size_t i = 0; i < xRanked.size(); ++i) {
        xRanks[xRanked[i].second] = static_cast<double>(i + 1);
    }
    
    // 计算y的秩
    for (size_t i = 0; i < y.size(); ++i) {
        yRanked.emplace_back(y[i], i);
    }
    std::sort(yRanked.begin(), yRanked.end());
    
    std::vector<double> yRanks(y.size());
    for (size_t i = 0; i < yRanked.size(); ++i) {
        yRanks[yRanked[i].second] = static_cast<double>(i + 1);
    }
    
    // 使用Pearson公式计算秩相关系数
    return pearsonCorrelation(xRanks, yRanks);
}

} // namespace ML
} // namespace Core
} // namespace BondForge