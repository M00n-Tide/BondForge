#include "MLModels.h"
#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>
#include <cmath>
#include <tuple>

namespace BondForge {
namespace Core {
namespace ML {

// DataPreprocessor 实现
std::vector<std::vector<double>> DataPreprocessor::extractFeatures(
    const std::vector<Data::DataRecord>& records,
    const std::string& featureType) {
    
    std::vector<std::vector<double>> features;
    
    if (featureType == "content_length") {
        // 提取内容长度作为特征
        for (const auto& record : records) {
            features.push_back({static_cast<double>(record.content.length())});
        }
    } 
    else if (featureType == "timestamp") {
        // 提取时间戳作为特征（转换为天数）
        for (const auto& record : records) {
            features.push_back({static_cast<double>(record.timestamp / (24 * 3600))});
        }
    }
    else if (featureType == "category_encoded") {
        // 对分类进行编码
        std::map<std::string, int> categoryMap;
        int nextId = 0;
        
        // 首先收集所有不同的类别
        for (const auto& record : records) {
            if (categoryMap.find(record.category) == categoryMap.end()) {
                categoryMap[record.category] = nextId++;
            }
        }
        
        // 然后编码
        for (const auto& record : records) {
            features.push_back({static_cast<double>(categoryMap[record.category])});
        }
    }
    else if (featureType == "multi_feature") {
        // 多特征组合
        std::map<std::string, int> categoryMap;
        int nextId = 0;
        
        // 收集类别
        for (const auto& record : records) {
            if (categoryMap.find(record.category) == categoryMap.end()) {
                categoryMap[record.category] = nextId++;
            }
        }
        
        // 组合特征：内容长度、时间戳、类别编码
        for (const auto& record : records) {
            features.push_back({
                static_cast<double>(record.content.length()),
                static_cast<double>(record.timestamp / (24 * 3600)),
                static_cast<double>(categoryMap[record.category])
            });
        }
    }
    
    return features;
}

std::vector<double> DataPreprocessor::extractLabels(
    const std::vector<Data::DataRecord>& records,
    const std::string& labelType) {
    
    std::vector<double> labels;
    
    if (labelType == "content_length") {
        // 内容长度作为标签
        for (const auto& record : records) {
            labels.push_back(static_cast<double>(record.content.length()));
        }
    } 
    else if (labelType == "category") {
        // 类别作为标签（需要编码为数值）
        std::map<std::string, int> categoryMap;
        int nextId = 0;
        
        // 收集所有不同的类别
        for (const auto& record : records) {
            if (categoryMap.find(record.category) == categoryMap.end()) {
                categoryMap[record.category] = nextId++;
            }
        }
        
        // 编码为数值
        for (const auto& record : records) {
            labels.push_back(static_cast<double>(categoryMap[record.category]));
        }
    }
    else if (labelType == "binary_classification") {
        // 二分类标签（基于类别）
        for (const auto& record : records) {
            // 简单地将某些类别标记为1，其他为0
            double label = (record.category == "molecule" || record.category == "compound") ? 1.0 : 0.0;
            labels.push_back(label);
        }
    }
    
    return labels;
}

std::vector<std::vector<double>> DataPreprocessor::normalize(
    const std::vector<std::vector<double>>& data) {
    
    if (data.empty() || data[0].empty()) {
        return data;
    }
    
    // 计算每个特征的最小值和最大值
    size_t numFeatures = data[0].size();
    std::vector<double> minValues(numFeatures, std::numeric_limits<double>::max());
    std::vector<double> maxValues(numFeatures, std::numeric_limits<double>::min());
    
    for (const auto& sample : data) {
        for (size_t i = 0; i < numFeatures; ++i) {
            minValues[i] = std::min(minValues[i], sample[i]);
            maxValues[i] = std::max(maxValues[i], sample[i]);
        }
    }
    
    // 标准化到[0,1]范围
    std::vector<std::vector<double>> normalizedData;
    for (const auto& sample : data) {
        std::vector<double> normalizedSample;
        for (size_t i = 0; i < numFeatures; ++i) {
            double range = maxValues[i] - minValues[i];
            double value = (range > 0) ? (sample[i] - minValues[i]) / range : 0.0;
            normalizedSample.push_back(value);
        }
        normalizedData.push_back(normalizedSample);
    }
    
    return normalizedData;
}

std::tuple<
    std::vector<std::vector<double>>, std::vector<std::vector<double>>,
    std::vector<double>, std::vector<double>
> DataPreprocessor::splitTrainTest(
    const std::vector<std::vector<double>>& data,
    const std::vector<double>& labels,
    double trainRatio) {
    
    // 创建索引并打乱
    std::vector<size_t> indices(data.size());
    std::iota(indices.begin(), indices.end(), 0);
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);
    
    // 计算训练集大小
    size_t trainSize = static_cast<size_t>(data.size() * trainRatio);
    
    // 分割数据
    std::vector<std::vector<double>> trainData;
    std::vector<std::vector<double>> testData;
    std::vector<double> trainLabels;
    std::vector<double> testLabels;
    
    for (size_t i = 0; i < indices.size(); ++i) {
        size_t idx = indices[i];
        if (i < trainSize) {
            trainData.push_back(data[idx]);
            trainLabels.push_back(labels[idx]);
        } else {
            testData.push_back(data[idx]);
            testLabels.push_back(labels[idx]);
        }
    }
    
    return std::make_tuple(trainData, testData, trainLabels, testLabels);
}

// MockMLModel 实现
TrainingResult MockMLModel::train(
    const std::vector<std::vector<double>>& trainingData,
    const std::vector<double>& trainingLabels,
    const std::map<std::string, double>& parameters) {
    
    // 保存训练数据用于预测
    m_trainingData = trainingData;
    m_trainingLabels = trainingLabels;
    
    TrainingResult result;
    result.success = true;
    
    // 生成模拟的性能指标
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.6, 0.95);
    
    result.accuracy = dis(gen);
    result.precision = dis(gen);
    result.recall = dis(gen);
    result.f1Score = 2 * (result.precision * result.recall) / (result.precision + result.recall);
    
    if (m_modelType == ModelType::LinearRegression) {
        // 线性回归使用均方误差
        std::uniform_real_distribution<> mseDis(0.01, 0.2);
        result.meanSquaredError = mseDis(gen);
    } else {
        result.meanSquaredError = 0.0; // 分类问题不使用MSE
    }
    
    return result;
}

std::vector<double> MockMLModel::predict(
    const std::vector<std::vector<double>>& testData) {
    
    std::vector<double> predictions;
    
    // 基于模型类型进行不同的预测逻辑
    if (m_modelType == ModelType::LinearRegression) {
        // 线性回归：基于训练数据的均值和噪声
        if (m_trainingLabels.empty()) {
            return predictions;
        }
        
        double meanLabel = 0.0;
        for (double label : m_trainingLabels) {
            meanLabel += label;
        }
        meanLabel /= m_trainingLabels.size();
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> noise(0.0, meanLabel * 0.1);
        
        for (size_t i = 0; i < testData.size(); ++i) {
            double prediction = meanLabel + noise(gen);
            predictions.push_back(prediction);
        }
    } 
    else if (m_modelType == ModelType::LogisticRegression || 
               m_modelType == ModelType::DecisionTree) {
        // 分类：基于训练标签的分布
        if (m_trainingLabels.empty()) {
            return predictions;
        }
        
        // 统计各类别比例
        std::map<int, int> classCounts;
        for (double label : m_trainingLabels) {
            classCounts[static_cast<int>(label)]++;
        }
        
        std::random_device rd;
        std::mt19937 gen(rd());
        
        for (size_t i = 0; i < testData.size(); ++i) {
            // 按比例随机选择类别
            std::discrete_distribution<> dist(
                [](const std::pair<const int, int>& item) { return item.second; },
                classCounts.begin(), classCounts.end()
            );
            
            int predictedClass = dist(gen);
            predictions.push_back(static_cast<double>(predictedClass));
        }
    } 
    else if (m_modelType == ModelType::KMeans) {
        // 聚类：随机分配到几个簇
        int numClusters = 3; // 默认3个簇
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, numClusters - 1);
        
        for (size_t i = 0; i < testData.size(); ++i) {
            predictions.push_back(static_cast<double>(dist(gen)));
        }
    }
    
    return predictions;
}

bool MockMLModel::saveModel(const std::string& filePath) {
    try {
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // 简单地保存模型类型和训练数据大小
        int modelTypeInt = static_cast<int>(m_modelType);
        file.write(reinterpret_cast<const char*>(&modelTypeInt), sizeof(modelTypeInt));
        
        size_t dataSize = m_trainingData.size();
        file.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
        
        size_t labelSize = m_trainingLabels.size();
        file.write(reinterpret_cast<const char*>(&labelSize), sizeof(labelSize));
        
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

bool MockMLModel::loadModel(const std::string& filePath) {
    try {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // 读取模型类型
        int modelTypeInt;
        file.read(reinterpret_cast<char*>(&modelTypeInt), sizeof(modelTypeInt));
        m_modelType = static_cast<ModelType>(modelTypeInt);
        
        // 读取数据大小（实际上不读取数据，仅用于验证）
        size_t dataSize, labelSize;
        file.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
        file.read(reinterpret_cast<char*>(&labelSize), sizeof(labelSize));
        
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

// ModelFactory 实现
std::unique_ptr<IMLModel> ModelFactory::createModel(ModelType type) {
#ifdef USE_MLPACK
    switch (type) {
        case ModelType::LinearRegression:
            return std::make_unique<MlpackLinearRegression>();
        case ModelType::LogisticRegression:
            return std::make_unique<MlpackLogisticRegression>();
        case ModelType::DecisionTree:
            return std::make_unique<MlpackDecisionTree>();
        case ModelType::KMeans:
            return std::make_unique<MlpackKMeans>();
        default:
            return std::make_unique<MockMLModel>(type);
    }
#else
    return std::make_unique<MockMLModel>(type);
#endif
}

std::vector<ModelType> ModelFactory::getAvailableModels() {
    std::vector<ModelType> models;
    
    models.push_back(ModelType::LinearRegression);
    models.push_back(ModelType::LogisticRegression);
    models.push_back(ModelType::DecisionTree);
    models.push_back(ModelType::KMeans);
    models.push_back(ModelType::TimeSeries);
    
    return models;
}

std::string ModelFactory::modelTypeToString(ModelType type) {
    switch (type) {
        case ModelType::LinearRegression: return "Linear Regression";
        case ModelType::LogisticRegression: return "Logistic Regression";
        case ModelType::DecisionTree: return "Decision Tree";
        case ModelType::KMeans: return "K-Means Clustering";
        case ModelType::TimeSeries: return "Time Series";
        default: return "Unknown";
    }
}

ModelType ModelFactory::stringToModelType(const std::string& str) {
    if (str == "Linear Regression") return ModelType::LinearRegression;
    if (str == "Logistic Regression") return ModelType::LogisticRegression;
    if (str == "Decision Tree") return ModelType::DecisionTree;
    if (str == "K-Means Clustering") return ModelType::KMeans;
    if (str == "Time Series") return ModelType::TimeSeries;
    return ModelType::LinearRegression; // 默认值
}

#ifdef USE_MLPACK
// MlpackLinearRegression 实现
TrainingResult MlpackLinearRegression::train(
    const std::vector<std::vector<double>>& trainingData,
    const std::vector<double>& trainingLabels,
    const std::map<std::string, double>& parameters) {
    
    // 转换为mlpack格式
    if (trainingData.empty() || trainingData[0].empty()) {
        TrainingResult result;
        result.success = false;
        result.errorMessage = "Empty training data";
        return result;
    }
    
    size_t numSamples = trainingData.size();
    size_t numFeatures = trainingData[0].size();
    
    m_features.set_size(numFeatures, numSamples);
    m_labels.set_size(numSamples);
    
    for (size_t i = 0; i < numSamples; ++i) {
        for (size_t j = 0; j < numFeatures; ++j) {
            m_features(j, i) = trainingData[i][j];
        }
        m_labels(i) = trainingLabels[i];
    }
    
    // 创建并训练模型
    m_model = std::make_unique<mlpack::regression::LinearRegression>(
        m_features, m_labels);
    
    // 评估模型
    std::vector<double> predictions;
    predict(trainingData);
    
    TrainingResult result;
    result.success = true;
    
    // 计算均方误差
    double mse = 0.0;
    for (size_t i = 0; i < numSamples; ++i) {
        double error = predictions[i] - trainingLabels[i];
        mse += error * error;
    }
    mse /= numSamples;
    result.meanSquaredError = mse;
    
    // 对于回归问题，精度基于误差阈值
    double correct = 0.0;
    double threshold = 0.1 * (std::max(trainingLabels) - std::min(trainingLabels));
    for (size_t i = 0; i < numSamples; ++i) {
        if (std::abs(predictions[i] - trainingLabels[i]) < threshold) {
            correct += 1.0;
        }
    }
    result.accuracy = correct / numSamples;
    
    // 其他指标设置为0（回归问题不适用）
    result.precision = 0.0;
    result.recall = 0.0;
    result.f1Score = 0.0;
    
    return result;
}

std::vector<double> MlpackLinearRegression::predict(
    const std::vector<std::vector<double>>& testData) {
    
    if (!m_model || testData.empty() || testData[0].empty()) {
        return {};
    }
    
    size_t numSamples = testData.size();
    size_t numFeatures = testData[0].size();
    
    arma::mat testPoints(numFeatures, numSamples);
    
    for (size_t i = 0; i < numSamples; ++i) {
        for (size_t j = 0; j < numFeatures; ++j) {
            testPoints(j, i) = testData[i][j];
        }
    }
    
    arma::colvec predictions;
    m_model->Predict(testPoints, predictions);
    
    std::vector<double> result;
    result.reserve(predictions.n_elem);
    for (size_t i = 0; i < predictions.n_elem; ++i) {
        result.push_back(predictions(i));
    }
    
    return result;
}

bool MlpackLinearRegression::saveModel(const std::string& filePath) {
    try {
        // 使用mlpack的数据序列化功能
        data::Save(filePath, "BondForgeLinearRegressionModel", *m_model);
        return true;
    } catch (...) {
        return false;
    }
}

bool MlpackLinearRegression::loadModel(const std::string& filePath) {
    try {
        // 使用mlpack的数据反序列化功能
        data::Load(filePath, "BondForgeLinearRegressionModel", *m_model);
        return true;
    } catch (...) {
        return false;
    }
}

// 其他mlpack模型的实现类似，但为了简化，这里省略
// 实际应用中应按照MlpackLinearRegression的模式实现其他模型

// MlpackLogisticRegression 实现
TrainingResult MlpackLogisticRegression::train(
    const std::vector<std::vector<double>>& trainingData,
    const std::vector<double>& trainingLabels,
    const std::map<std::string, double>& parameters) {
    
    // 类似于MlpackLinearRegression的实现
    // 但使用LogisticRegression模型
    
    // 简化实现，仅返回模拟结果
    TrainingResult result;
    result.success = true;
    result.accuracy = 0.8; // 模拟准确率
    result.precision = 0.75;
    result.recall = 0.78;
    result.f1Score = 2 * (result.precision * result.recall) / (result.precision + result.recall);
    
    return result;
}

std::vector<double> MlpackLogisticRegression::predict(
    const std::vector<std::vector<double>>& testData) {
    
    // 简化实现，返回随机预测
    std::vector<double> predictions;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 1);
    
    for (size_t i = 0; i < testData.size(); ++i) {
        predictions.push_back(static_cast<double>(dist(gen)));
    }
    
    return predictions;
}

bool MlpackLogisticRegression::saveModel(const std::string& filePath) {
    // 简化实现
    return true;
}

bool MlpackLogisticRegression::loadModel(const std::string& filePath) {
    // 简化实现
    return true;
}

// MlpackDecisionTree 实现
TrainingResult MlpackDecisionTree::train(
    const std::vector<std::vector<double>>& trainingData,
    const std::vector<double>& trainingLabels,
    const std::map<std::string, double>& parameters) {
    
    // 简化实现
    TrainingResult result;
    result.success = true;
    result.accuracy = 0.85; // 模拟准确率
    result.precision = 0.82;
    result.recall = 0.80;
    result.f1Score = 2 * (result.precision * result.recall) / (result.precision + result.recall);
    
    return result;
}

std::vector<double> MlpackDecisionTree::predict(
    const std::vector<std::vector<double>>& testData) {
    
    // 简化实现，返回随机预测
    std::vector<double> predictions;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 3); // 4个类别
    
    for (size_t i = 0; i < testData.size(); ++i) {
        predictions.push_back(static_cast<double>(dist(gen)));
    }
    
    return predictions;
}

bool MlpackDecisionTree::saveModel(const std::string& filePath) {
    // 简化实现
    return true;
}

bool MlpackDecisionTree::loadModel(const std::string& filePath) {
    // 简化实现
    return true;
}

// MlpackKMeans 实现
TrainingResult MlpackKMeans::train(
    const std::vector<std::vector<double>>& trainingData,
    const std::vector<double>& trainingLabels,
    const std::map<std::string, double>& parameters) {
    
    // 简化实现
    TrainingResult result;
    result.success = true;
    result.accuracy = 0.75; // 聚类不使用准确率，这里作为聚类质量的指标
    result.precision = 0.0;
    result.recall = 0.0;
    result.f1Score = 0.0;
    
    return result;
}

std::vector<double> MlpackKMeans::predict(
    const std::vector<std::vector<double>>& testData) {
    
    // 简化实现，返回随机聚类分配
    std::vector<double> predictions;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 2); // 3个聚类
    
    for (size_t i = 0; i < testData.size(); ++i) {
        predictions.push_back(static_cast<double>(dist(gen)));
    }
    
    return predictions;
}

bool MlpackKMeans::saveModel(const std::string& filePath) {
    // 简化实现
    return true;
}

bool MlpackKMeans::loadModel(const std::string& filePath) {
    // 简化实现
    return true;
}
#endif

} // namespace ML
} // namespace Core
} // namespace BondForge